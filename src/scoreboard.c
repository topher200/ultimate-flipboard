#include "scoreboard.h"

void scoreboard_init(scoreboard_t *sb)
{
	sb->score_a = 0;
	sb->score_b = 0;
}

void scoreboard_inc_a(scoreboard_t *sb) { sb->score_a++; }
void scoreboard_inc_b(scoreboard_t *sb) { sb->score_b++; }

void scoreboard_dec_a(scoreboard_t *sb)
{
	if (sb->score_a > 0) sb->score_a--;
}

void scoreboard_dec_b(scoreboard_t *sb)
{
	if (sb->score_b > 0) sb->score_b--;
}

void scoreboard_reset(scoreboard_t *sb)
{
	sb->score_a = 0;
	sb->score_b = 0;
}
