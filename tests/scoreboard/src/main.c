#include <zephyr/ztest.h>
#include "scoreboard.h"

ZTEST_SUITE(scoreboard, NULL, NULL, NULL, NULL, NULL);

ZTEST(scoreboard, test_init_zeros)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	zassert_equal(sb.score_a, 0);
	zassert_equal(sb.score_b, 0);
}

ZTEST(scoreboard, test_inc_a_only_affects_a)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	scoreboard_inc_a(&sb);
	zassert_equal(sb.score_a, 1);
	zassert_equal(sb.score_b, 0, "inc_a must not touch score_b");
}

ZTEST(scoreboard, test_inc_b_only_affects_b)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	scoreboard_inc_b(&sb);
	zassert_equal(sb.score_a, 0, "inc_b must not touch score_a");
	zassert_equal(sb.score_b, 1);
}

ZTEST(scoreboard, test_inc_accumulates)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	for (int i = 0; i < 10; i++) {
		scoreboard_inc_a(&sb);
	}
	for (int i = 0; i < 7; i++) {
		scoreboard_inc_b(&sb);
	}
	zassert_equal(sb.score_a, 10);
	zassert_equal(sb.score_b, 7);
}

ZTEST(scoreboard, test_dec_a_floors_at_zero)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	scoreboard_dec_a(&sb); /* already 0 — must stay 0 */
	zassert_equal(sb.score_a, 0, "dec below zero must be a no-op");

	scoreboard_inc_a(&sb);
	scoreboard_inc_a(&sb);
	scoreboard_dec_a(&sb);
	zassert_equal(sb.score_a, 1);
}

ZTEST(scoreboard, test_dec_b_floors_at_zero)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	scoreboard_dec_b(&sb);
	zassert_equal(sb.score_b, 0, "dec below zero must be a no-op");

	scoreboard_inc_b(&sb);
	scoreboard_inc_b(&sb);
	scoreboard_dec_b(&sb);
	zassert_equal(sb.score_b, 1);
}

ZTEST(scoreboard, test_reset_clears_both)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	scoreboard_inc_a(&sb);
	scoreboard_inc_a(&sb);
	scoreboard_inc_b(&sb);
	scoreboard_reset(&sb);
	zassert_equal(sb.score_a, 0);
	zassert_equal(sb.score_b, 0);
}

ZTEST(scoreboard, test_dec_only_affects_own_side)
{
	scoreboard_t sb;

	scoreboard_init(&sb);
	scoreboard_inc_a(&sb);
	scoreboard_inc_b(&sb);
	scoreboard_dec_a(&sb);
	zassert_equal(sb.score_a, 0);
	zassert_equal(sb.score_b, 1, "dec_a must not touch score_b");
}
