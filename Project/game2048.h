#ifndef __GAME2048_H
#define __GAME2048_H

#include <stdint.h>
#include "game2048_core.h"

#define SIZE GAME2048_BOARD_SIZE

#define GAME_DIFF_EASY    0
#define GAME_DIFF_NORMAL  1
#define GAME_DIFF_HARD    2

void Game_SetDifficulty(uint8_t diff);
uint8_t Game_GetDifficulty(void);
void Game_Init(void);
void Game_GenerateNumber(void);
uint8_t Game_Move(uint8_t dir);
uint8_t Game_IsWin(void);
void Game_ContinueAfterWin(void);
uint8_t Game_IsLose(void);
void Game_ShowBoard(void);
uint32_t Game_GetScore(void);

#endif
