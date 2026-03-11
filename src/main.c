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

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

/* ── Score state ─────────────────────────────────────────────────────────── */

static scoreboard_t sb;

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
	LOG_INF("Ultimate Flipboard starting");

	scoreboard_init(&sb);

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
