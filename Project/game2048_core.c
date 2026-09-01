#include "game2048_core.h"

uint8_t Game2048_ProcessLine(uint16_t line[GAME2048_BOARD_SIZE],
                             uint32_t *score_delta,
                             uint8_t *reached_2048)
{
    uint16_t compact[GAME2048_BOARD_SIZE];
    uint16_t result[GAME2048_BOARD_SIZE] = {0U};
    uint8_t count = 0U;
    uint8_t read;
    uint8_t write = 0U;
    uint8_t changed = 0U;
    uint32_t added_score = 0U;

    if (score_delta != 0) *score_delta = 0U;
    if (reached_2048 != 0) *reached_2048 = 0U;

    for (read = 0U; read < GAME2048_BOARD_SIZE; read++) {
        if (line[read] != 0U) compact[count++] = line[read];
    }

    read = 0U;
    while (read < count) {
        uint16_t value = compact[read++];
        if (read < count && value == compact[read]) {
            value = (uint16_t)(value * 2U);
            added_score += value;
            if (value >= 2048U && reached_2048 != 0) *reached_2048 = 1U;
            read++;
        }
        result[write++] = value;
    }

    for (read = 0U; read < GAME2048_BOARD_SIZE; read++) {
        if (line[read] != result[read]) changed = 1U;
        line[read] = result[read];
    }

    if (score_delta != 0) *score_delta = added_score;
    return changed;
}
