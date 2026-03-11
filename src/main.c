/*
 * Ultimate Flipboard — APP core (nRF54H20 cpuapp)
 *
 * Reads four GPIO buttons and maintains a two-team scoreboard.
 * Output goes to the console (RTT or UART) for now; the flipdots
 * display driver will replace print_score() later.
 *
 * Button layout (nRF54H20DK):
 *   BTN1 (sw0) — Team A +1
 *   BTN2 (sw1) — Team B +1
 *   BTN3 (sw2) — Undo: Team A −1
 *   BTN4 (sw3) — Reset both scores
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "scoreboard.h"
#include "flipdots.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* ── Score state ─────────────────────────────────────────────────────────── */

static scoreboard_t sb;

/* ── Flipdots test mode ──────────────────────────────────────────────────── */

#define TEST_MODE_INTERVAL_MS 300  /* ~3.3 Hz flip rate */

static flipdots_fb_t test_fb;
static bool test_is_white;
static struct k_work_delayable test_work;

static void test_mode_update(struct k_work *work)
{
	ARG_UNUSED(work);

	/* Toggle state */
	test_is_white = !test_is_white;

	/* Fill or clear framebuffer */
	if (test_is_white) {
		flipdots_fill(test_fb);
	} else {
		flipdots_clear(test_fb);
	}

	/* Send frame (broadcast address 0xFF) */
	int ret = flipdots_send_frame(0xFF, test_fb, true);
	if (ret != 0) {
		LOG_WRN("Failed to send flipdots frame: %d", ret);
	}

	/* Reschedule for next flip */
	k_work_schedule(&test_work, K_MSEC(TEST_MODE_INTERVAL_MS));
}

static void print_score(void)
{
	LOG_INF("Score │ Team A: %2d  │  Team B: %2d", sb.score_a, sb.score_b);
	/* TODO: drive the flipdots display here */
}

/* ── Button definitions ──────────────────────────────────────────────────── */

static const struct gpio_dt_spec btn_inc_a = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static const struct gpio_dt_spec btn_inc_b = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
static const struct gpio_dt_spec btn_dec_a = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
static const struct gpio_dt_spec btn_reset = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);

/* ── Debounce ────────────────────────────────────────────────────────────── */

#define DEBOUNCE_MS 50

struct btn_state { int64_t last_ms; };
static struct btn_state st_inc_a, st_inc_b, st_dec_a, st_reset;

static bool debounce(struct btn_state *s)
{
	int64_t now = k_uptime_get();

	if ((now - s->last_ms) < DEBOUNCE_MS) {
		return false;
	}
	s->last_ms = now;
	return true;
}

/* ── GPIO callbacks ──────────────────────────────────────────────────────── */

static struct gpio_callback cb_inc_a, cb_inc_b, cb_dec_a, cb_reset;

static void on_inc_a(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	if (!debounce(&st_inc_a)) return;
	scoreboard_inc_a(&sb);
	print_score();
}

static void on_inc_b(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	if (!debounce(&st_inc_b)) return;
	scoreboard_inc_b(&sb);
	print_score();
}

static void on_dec_a(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	if (!debounce(&st_dec_a)) return;
	scoreboard_dec_a(&sb);
	print_score();
}

static void on_reset(const struct device *dev, struct gpio_callback *cb,
		     uint32_t pins)
{
	ARG_UNUSED(dev); ARG_UNUSED(cb); ARG_UNUSED(pins);
	if (!debounce(&st_reset)) return;
	scoreboard_reset(&sb);
	LOG_INF("Scoreboard reset");
	print_score();
}

/* ── Setup helper ────────────────────────────────────────────────────────── */

static int setup_button(const struct gpio_dt_spec *btn,
			struct gpio_callback *cb,
			gpio_callback_handler_t handler)
{
	int ret;

	if (!gpio_is_ready_dt(btn)) {
		LOG_ERR("GPIO not ready: %s", btn->port->name);
		return -ENODEV;
	}
	ret = gpio_pin_configure_dt(btn, GPIO_INPUT);
	if (ret) return ret;

	ret = gpio_pin_interrupt_configure_dt(btn, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret) return ret;

	gpio_init_callback(cb, handler, BIT(btn->pin));
	gpio_add_callback(btn->port, cb);
	return 0;
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

int main(void)
{
	int ret;

	LOG_INF("Ultimate Flipboard starting");

	scoreboard_init(&sb);

	/* Initialize flipdots driver */
	ret = flipdots_init();
	if (ret == 0) {
		/* Start automatic test mode */
		test_is_white = false;
		k_work_init_delayable(&test_work, test_mode_update);
		k_work_schedule(&test_work, K_MSEC(TEST_MODE_INTERVAL_MS));
		LOG_INF("Flipdots test mode started");
	} else {
		LOG_WRN("Flipdots initialization failed: %d (test mode disabled)", ret);
	}

	setup_button(&btn_inc_a, &cb_inc_a, on_inc_a);
	setup_button(&btn_inc_b, &cb_inc_b, on_inc_b);
	setup_button(&btn_dec_a, &cb_dec_a, on_dec_a);
	setup_button(&btn_reset, &cb_reset, on_reset);

	LOG_INF("Ready — BTN1=A+  BTN2=B+  BTN3=A−  BTN4=reset");
	print_score();

	while (true) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
