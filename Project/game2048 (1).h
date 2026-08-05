#ifndef __GAME2048_H
#define __GAME2048_H

#include <stdint.h>

#define SIZE 4

void Game_Init(void);
void Game_GenerateNumber(void);
uint8_t Game_Move(uint8_t dir);
uint8_t Game_IsWin(void);
uint8_t Game_IsLose(void);
void Game_ShowBoard(void);

#endif