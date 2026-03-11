/*
 * Ultimate Flipboard — PPR core (nRF54H20 cpuppr)
 *
 * Monitors GPIO buttons and sends score events to the APP core over IPC.
 *
 * Button wiring (nRF54H20DK defaults):
 *   sw0 (Button 1) → Team A scores
 *   sw1 (Button 2) → Team B scores
 *   sw2 (Button 3) → Undo last Team A point
 *   sw3 (Button 4) → Reset scoreboard
 *
 * If a button is not present on your hardware, the build will fail with a
 * "node not found" error — just remove that alias from setup_buttons().
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/ipc/ipc_service.h>
#include <zephyr/logging/log.h>

#include "scoreboard_events.h"

LOG_MODULE_REGISTER(ppr, LOG_LEVEL_INF);

/* ── Button definitions (from board device tree aliases) ─────────────────── */

static const struct gpio_dt_spec btn_a     = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec btn_b     = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec btn_dec_a = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec btn_reset = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);

/* ── Message queue (ISR → sender thread) ────────────────────────────────── */

/* ISRs enqueue here; the sender thread drains it and calls ipc_service_send.
 * ipc_service_send must NOT be called from interrupt context. */
K_MSGQ_DEFINE(score_msgq, sizeof(struct score_event), 16, 4);

/* ── Button debounce ─────────────────────────────────────────────────────── */

#define DEBOUNCE_MS 50

struct btn_state {
	int64_t last_press_ms;
};

static struct btn_state state_a, state_b, state_dec_a, state_reset;

static void enqueue_if_debounced(struct btn_state *s, enum score_cmd cmd)
{
	int64_t now = k_uptime_get();

	if ((now - s->last_press_ms) < DEBOUNCE_MS) {
		return; /* bounce — ignore */
	}
	s->last_press_ms = now;

	struct score_event evt = { .cmd = (uint8_t)cmd };

	if (k_msgq_put(&score_msgq, &evt, K_NO_WAIT) != 0) {
		/* Queue full — shouldn't happen in normal use */
	}
}

/* ── GPIO callbacks (interrupt context) ─────────────────────────────────── */

static struct gpio_callback cb_a, cb_b, cb_dec_a, cb_reset;

static void on_btn_a(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	enqueue_if_debounced(&state_a, SCORE_INC_A);
}

static void on_btn_b(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	enqueue_if_debounced(&state_b, SCORE_INC_B);
}

static void on_btn_dec_a(const struct device *dev, struct gpio_callback *cb,
			 uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	enqueue_if_debounced(&state_dec_a, SCORE_DEC_A);
}

static void on_btn_reset(const struct device *dev, struct gpio_callback *cb,
			 uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	enqueue_if_debounced(&state_reset, SCORE_RESET);
}

/* ── GPIO setup helpers ──────────────────────────────────────────────────── */

static int setup_button(const struct gpio_dt_spec *btn,
			struct gpio_callback *cb,
			gpio_callback_handler_t handler)
{
	int ret;

	if (!gpio_is_ready_dt(btn)) {
		LOG_ERR("GPIO device %s not ready", btn->port->name);
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(btn, GPIO_INPUT);
	if (ret) {
		LOG_ERR("gpio_pin_configure_dt failed: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(btn, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret) {
		LOG_ERR("gpio_pin_interrupt_configure_dt failed: %d", ret);
		return ret;
	}

	gpio_init_callback(cb, handler, BIT(btn->pin));
	gpio_add_callback(btn->port, cb);

	return 0;
}

static void setup_buttons(void)
{
	setup_button(&btn_a,     &cb_a,     on_btn_a);
	setup_button(&btn_b,     &cb_b,     on_btn_b);
	setup_button(&btn_dec_a, &cb_dec_a, on_btn_dec_a);
	setup_button(&btn_reset, &cb_reset, on_btn_reset);
}

/* ── IPC setup ───────────────────────────────────────────────────────────── */

static K_SEM_DEFINE(ipc_bound_sem, 0, 1);
static struct ipc_ept ep;

static void on_ipc_bound(void *priv)
{
	LOG_INF("IPC bound — APP core connected");
	k_sem_give(&ipc_bound_sem);
}

static const struct ipc_ept_cfg ep_cfg = {
	.name = "scoreboard",
	.cb = {
		.bound = on_ipc_bound,
		/* PPR only sends; no .received handler needed */
	},
};

/* ── Entry point ─────────────────────────────────────────────────────────── */

int main(void)
{
	const struct device *ipc0 = DEVICE_DT_GET(DT_NODELABEL(ipc0));
	int ret;

	LOG_INF("Ultimate Flipboard — PPR core starting");

	ret = ipc_service_open_instance(ipc0);
	if (ret < 0 && ret != -EALREADY) {
		LOG_ERR("ipc_service_open_instance failed: %d", ret);
		return ret;
	}

	ret = ipc_service_register_endpoint(ipc0, &ep, &ep_cfg);
	if (ret < 0) {
		LOG_ERR("ipc_service_register_endpoint failed: %d", ret);
		return ret;
	}

	/* Block until the APP core registers its side of the endpoint. */
	k_sem_take(&ipc_bound_sem, K_FOREVER);

	setup_buttons();
	LOG_INF("PPR ready — btn1=A+, btn2=B+, btn3=A−, btn4=reset");

	/* Drain the message queue and forward events to APP core. */
	while (true) {
		struct score_event evt;

		k_msgq_get(&score_msgq, &evt, K_FOREVER);

		ret = ipc_service_send(&ep, &evt, sizeof(evt));
		if (ret < 0) {
			LOG_WRN("ipc_service_send failed: %d", ret);
		}
	}

	return 0;
}
