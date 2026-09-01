#ifndef __GAME2048_CORE_H
#define __GAME2048_CORE_H

#include <stdint.h>

#define GAME2048_BOARD_SIZE 4U

/* Hardware-free line merge routine, shared by firmware and host tests. */
uint8_t Game2048_ProcessLine(uint16_t line[GAME2048_BOARD_SIZE],
                             uint32_t *score_delta,
                             uint8_t *reached_2048);

#endif /* __GAME2048_CORE_H */
