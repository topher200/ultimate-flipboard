/*
 * Flipdots 7×28 display driver implementation
 *
 * Uses GPIO bit-bang UART TX on P9.04 because the nRF54H20 UARTE
 * peripherals cannot route output to GPIO port 9 pins (confirmed
 * by testing uart120, uart130, uart131, uart136 — all produce a
 * flat line on P9, while GPIO toggle works fine at 3.3 V).
 *
 * 9600 baud, 8-N-1, via RS-485 breakout board.
 */

#include "flipdots.h"

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flipdots, LOG_LEVEL_DBG);

/* ── Software UART TX on P9.04 ──────────────────────────────────────────── */

#define SW_UART_TX_PIN   4          /* P9.04 */
#define SW_UART_BIT_US   104        /* 1 000 000 / 9600 ≈ 104.17 µs */

static const struct device *gpio9_dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpio9));

/**
 * Transmit one byte using bit-bang UART (8-N-1, LSB first).
 * Interrupts are locked for the duration (~1.04 ms) to keep
 * timing within the ±3 % tolerance that UART receivers allow.
 */
static void sw_uart_tx_byte(uint8_t byte)
{
	unsigned int key = irq_lock();

	/* Start bit — LOW */
	gpio_pin_set_raw(gpio9_dev, SW_UART_TX_PIN, 0);
	k_busy_wait(SW_UART_BIT_US);

	/* 8 data bits, LSB first */
	for (int i = 0; i < 8; i++) {
		gpio_pin_set_raw(gpio9_dev, SW_UART_TX_PIN, (byte >> i) & 1);
		k_busy_wait(SW_UART_BIT_US);
	}

	/* Stop bit — HIGH */
	gpio_pin_set_raw(gpio9_dev, SW_UART_TX_PIN, 1);
	k_busy_wait(SW_UART_BIT_US);

	irq_unlock(key);
}

/* ── Protocol constants ─────────────────────────────────────────────────── */

#define FRAME_START 0x80
#define FRAME_END   0x8F
#define CMD_7X28_REFRESH 0x83
#define CMD_7X28_BUFFERED 0x84
#define CMD_REFRESH_ALL 0x82

#define DISPLAY_WIDTH 28
#define DISPLAY_HEIGHT 7

/* ── Framebuffer helpers ────────────────────────────────────────────────── */

void flipdots_clear(flipdots_fb_t buf)
{
	memset(buf, 0, DISPLAY_WIDTH);
}

void flipdots_fill(flipdots_fb_t buf)
{
	/* Set bits 0-6 (all 7 dots visible), bit 7 must be 0 */
	memset(buf, 0x7F, DISPLAY_WIDTH);
}

void flipdots_set_pixel(flipdots_fb_t buf, uint8_t x, uint8_t y, bool on)
{
	if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
		return;
	}

	if (on) {
		buf[x] |= (1u << y);
	} else {
		buf[x] &= ~(1u << y);
	}

	/* Ensure bit 7 is always 0 */
	buf[x] &= 0x7F;
}

/* ── Frame TX ───────────────────────────────────────────────────────────── */

static int send_frame_bytes(const uint8_t *data, size_t len)
{
	if (gpio9_dev == NULL || !device_is_ready(gpio9_dev)) {
		LOG_ERR("GPIO9 device not ready");
		return -ENODEV;
	}

	LOG_HEXDUMP_DBG(data, len, "TX frame");

	for (size_t i = 0; i < len; i++) {
		sw_uart_tx_byte(data[i]);
	}

	return 0;
}

int flipdots_send_frame(uint8_t addr, const flipdots_fb_t buf, bool refresh_now)
{
	uint8_t frame[1 + 1 + 1 + DISPLAY_WIDTH + 1]; /* start + cmd + addr + data + end */
	size_t idx = 0;
	uint8_t cmd;

	if (gpio9_dev == NULL || !device_is_ready(gpio9_dev)) {
		LOG_ERR("GPIO9 device not ready");
		return -ENODEV;
	}

	/* Build frame */
	frame[idx++] = FRAME_START;

	if (refresh_now) {
		cmd = CMD_7X28_REFRESH;
	} else {
		cmd = CMD_7X28_BUFFERED;
	}
	frame[idx++] = cmd;
	frame[idx++] = addr;

	/* Copy data bytes */
	memcpy(&frame[idx], buf, DISPLAY_WIDTH);
	idx += DISPLAY_WIDTH;

	frame[idx++] = FRAME_END;

	return send_frame_bytes(frame, idx);
}

/* ── Init ───────────────────────────────────────────────────────────────── */

int flipdots_init(void)
{
	if (gpio9_dev == NULL) {
		LOG_ERR("Flipdots GPIO9 device not found");
		return -ENODEV;
	}

	if (!device_is_ready(gpio9_dev)) {
		LOG_ERR("Flipdots GPIO9 device not ready");
		return -ENODEV;
	}

	/* Configure TX pin: output, idle HIGH (UART idle state) */
	int ret = gpio_pin_configure(gpio9_dev, SW_UART_TX_PIN, GPIO_OUTPUT_HIGH);
	if (ret) {
		LOG_ERR("Failed to configure P9.%02d: %d", SW_UART_TX_PIN, ret);
		return ret;
	}

	LOG_INF("Flipdots driver initialized (bit-bang UART on P9.%02d, 9600 baud)",
		SW_UART_TX_PIN);
	return 0;
}
