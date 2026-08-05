/*========================= game.h =========================*/

#ifndef __GAME_H
#define __GAME_H

#include "gd32f4xx.h"

#define KEY_UP      1
#define KEY_DOWN    2
#define KEY_LEFT    3
#define KEY_RIGHT   4

extern int board[4][4];

void game_init(void);
void game_display(void);

void move_left(void);
void move_right(void);
void move_up(void);
void move_down(void);

void key_init(void);
int key_scan(void);

#endif