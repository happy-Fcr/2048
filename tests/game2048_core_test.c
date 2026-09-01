#include "../Project/game2048_core.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void check_line(const uint16_t input[GAME2048_BOARD_SIZE],
                       const uint16_t expected[GAME2048_BOARD_SIZE],
                       uint32_t expected_score, uint8_t expected_win)
{
    uint16_t line[GAME2048_BOARD_SIZE];
    uint32_t score;
    uint8_t reached;
    uint8_t changed;
    uint8_t i;

    for (i = 0U; i < GAME2048_BOARD_SIZE; i++) line[i] = input[i];
    changed = Game2048_ProcessLine(line, &score, &reached);
    assert(changed != 0U);
    assert(score == expected_score);
    assert(reached == expected_win);
    for (i = 0U; i < GAME2048_BOARD_SIZE; i++) assert(line[i] == expected[i]);
}

int main(void)
{
    static const uint16_t all_pairs[] = {2U, 2U, 2U, 2U};
    static const uint16_t all_pairs_out[] = {4U, 4U, 0U, 0U};
    static const uint16_t gap_merge[] = {2U, 0U, 2U, 2U};
    static const uint16_t gap_merge_out[] = {4U, 2U, 0U, 0U};
    static const uint16_t triple[] = {4U, 4U, 4U, 0U};
    static const uint16_t triple_out[] = {8U, 4U, 0U, 0U};
    static const uint16_t mixed[] = {2U, 2U, 4U, 4U};
    static const uint16_t mixed_out[] = {4U, 8U, 0U, 0U};
    static const uint16_t high[] = {1024U, 1024U, 0U, 0U};
    static const uint16_t high_out[] = {2048U, 0U, 0U, 0U};
    uint16_t unchanged[] = {2U, 4U, 8U, 16U};
    uint16_t empty[] = {0U, 0U, 0U, 0U};
    uint32_t score;
    uint8_t reached;

    check_line(all_pairs, all_pairs_out, 8U, 0U);
    check_line(gap_merge, gap_merge_out, 4U, 0U);
    check_line(triple, triple_out, 8U, 0U);
    check_line(mixed, mixed_out, 12U, 0U);
    check_line(high, high_out, 2048U, 1U);
    assert(Game2048_ProcessLine(unchanged, &score, &reached) == 0U);
    assert(score == 0U && reached == 0U);
    assert(Game2048_ProcessLine(empty, &score, &reached) == 0U);
    assert(score == 0U && reached == 0U);
    puts("game2048 core tests passed");
    return 0;
}
