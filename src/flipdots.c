/*
 * Flipdots 7x28 display driver implementation
 */

#include "flipdots.h"

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flipdots, LOG_LEVEL_INF);

/* UART device for RS485 communication */
#define FLIPDOTS_UART_NODE DT_ALIAS(flipdots_serial)
static const struct device *uart_dev = DEVICE_DT_GET_OR_NULL(FLIPDOTS_UART_NODE);

/* Protocol constants */
#define FRAME_START 0x80
#define FRAME_END   0x8F
#define CMD_7X28_REFRESH 0x83
#define CMD_7X28_BUFFERED 0x84
#define CMD_REFRESH_ALL 0x82

#define DISPLAY_WIDTH 28
#define DISPLAY_HEIGHT 7

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

static int send_frame_bytes(const uint8_t *data, size_t len)
{
	if (uart_dev == NULL || !device_is_ready(uart_dev)) {
		LOG_ERR("UART device not ready");
		return -ENODEV;
	}

	for (size_t i = 0; i < len; i++) {
		uart_poll_out(uart_dev, data[i]);
	}

	return 0;
}

int flipdots_send_frame(uint8_t addr, const flipdots_fb_t buf, bool refresh_now)
{
	uint8_t frame[1 + 1 + 1 + DISPLAY_WIDTH + 1]; /* start + cmd + addr + data + end */
	size_t idx = 0;
	uint8_t cmd;

	if (uart_dev == NULL || !device_is_ready(uart_dev)) {
		LOG_ERR("UART device not ready");
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

int flipdots_init(void)
{
	if (uart_dev == NULL) {
		LOG_ERR("Flipdots UART device not found (check DT_ALIAS(flipdots_serial))");
		return -ENODEV;
	}

	if (!device_is_ready(uart_dev)) {
		LOG_ERR("Flipdots UART device not ready");
		return -ENODEV;
	}

	LOG_INF("Flipdots driver initialized");
	return 0;
}
