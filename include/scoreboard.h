/*
 * Scoreboard state machine — no Zephyr or hardware dependencies.
 * This lives here so it can be unit-tested independently of main.c.
 */

#pragma once

typedef struct {
	int score_a;
	int score_b;
} scoreboard_t;

void scoreboard_init(scoreboard_t *sb);
void scoreboard_inc_a(scoreboard_t *sb);
void scoreboard_inc_b(scoreboard_t *sb);
void scoreboard_dec_a(scoreboard_t *sb);  /* floors at 0 */
void scoreboard_dec_b(scoreboard_t *sb);  /* floors at 0 */
void scoreboard_reset(scoreboard_t *sb);
