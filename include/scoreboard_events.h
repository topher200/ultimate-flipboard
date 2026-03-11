/*
 * Shared IPC message definitions between APP core and PPR core.
 * Keep this file dependency-free so it can also be used in host-side tests.
 */

#pragma once

#include <stdint.h>

enum score_cmd {
	SCORE_INC_A = 0, /* Team A scored a point      */
	SCORE_INC_B = 1, /* Team B scored a point      */
	SCORE_DEC_A = 2, /* Undo: decrement Team A     */
	SCORE_DEC_B = 3, /* Undo: decrement Team B     */
	SCORE_RESET = 4, /* Reset both scores to zero  */
};

/* Fixed-size IPC payload (4 bytes, naturally aligned). */
struct score_event {
	uint8_t cmd;         /* enum score_cmd              */
	uint8_t reserved[3]; /* pad to 4 bytes, must be 0  */
};
