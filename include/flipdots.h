/*
 * Flipdots 7x28 display driver
 *
 * Provides a framebuffer API for controlling a 7×28 AlfaZeta flipdots panel
 * over RS485 serial protocol.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/* 7×28 framebuffer: 28 bytes, one per column */
typedef uint8_t flipdots_fb_t[28];

/* Clear all dots (all black/hidden) */
void flipdots_clear(flipdots_fb_t buf);

/* Fill all dots (all white/visible) */
void flipdots_fill(flipdots_fb_t buf);

/* Set individual pixel state */
void flipdots_set_pixel(flipdots_fb_t buf, uint8_t x, uint8_t y, bool on);

/* Send frame to display over serial */
int flipdots_send_frame(uint8_t addr, const flipdots_fb_t buf, bool refresh_now);

/* Initialize the flipdots driver (must be called before sending frames) */
int flipdots_init(void);
