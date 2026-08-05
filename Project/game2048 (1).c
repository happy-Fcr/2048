#include "game2048.h"
#include "lcd_driver.h"
#include "key.h"
#include <stdlib.h>

static uint16_t board[SIZE][SIZE];
static uint8_t win_flag = 0;

void Game_Init(void)
{
    uint8_t i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            board[i][j] = 0;
        }
    }
    win_flag = 0;
    Game_GenerateNumber();
    Game_GenerateNumber();
}

void Game_GenerateNumber(void)
{
    uint8_t empty_cells[SIZE*SIZE][2];
    uint8_t empty_count = 0;
    uint8_t i, j, pos, value;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (board[i][j] == 0) {
                empty_cells[empty_count][0] = i;
                empty_cells[empty_count][1] = j;
                empty_count++;
            }
        }
    }
    if (empty_count == 0) return;
    pos = rand() % empty_count;
    value = (rand() % 10) ? 2 : 4;
    board[empty_cells[pos][0]][empty_cells[pos][1]] = value;
}

static uint8_t move_up(void)
{
    uint8_t moved = 0;
    int i, j, k;   // 使用有符号整数避免无限循环
    for (j = 0; j < SIZE; j++) {
        // 合并
        for (i = 0; i < SIZE - 1; i++) {
            if (board[i][j] == 0) continue;
            for (k = i+1; k < SIZE; k++) {
                if (board[k][j] == 0) continue;
                if (board[i][j] == board[k][j]) {
                    board[i][j] *= 2;
                    if (board[i][j] == 2048) win_flag = 1;
                    board[k][j] = 0;
                    moved = 1;
                }
                break;
            }
        }
        // 移动
        for (i = 0; i < SIZE; i++) {
            if (board[i][j] == 0) {
                for (k = i+1; k < SIZE; k++) {
                    if (board[k][j] != 0) {
                        board[i][j] = board[k][j];
                        board[k][j] = 0;
                        moved = 1;
                        break;
                    }
                }
            }
        }
    }
    return moved;
}

static uint8_t move_down(void)
{
    uint8_t moved = 0;
    int i, j, k;
    for (j = 0; j < SIZE; j++) {
        for (i = SIZE-1; i > 0; i--) {
            if (board[i][j] == 0) continue;
            for (k = i-1; k >= 0; k--) {
                if (board[k][j] == 0) continue;
                if (board[i][j] == board[k][j]) {
                    board[i][j] *= 2;
                    if (board[i][j] == 2048) win_flag = 1;
                    board[k][j] = 0;
                    moved = 1;
                }
                break;
            }
        }
        for (i = SIZE-1; i >= 0; i--) {
            if (board[i][j] == 0) {
                for (k = i-1; k >= 0; k--) {
                    if (board[k][j] != 0) {
                        board[i][j] = board[k][j];
                        board[k][j] = 0;
                        moved = 1;
                        break;
                    }
                }
            }
        }
    }
    return moved;
}

static uint8_t move_left(void)
{
    uint8_t moved = 0;
    int i, j, k;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE-1; j++) {
            if (board[i][j] == 0) continue;
            for (k = j+1; k < SIZE; k++) {
                if (board[i][k] == 0) continue;
                if (board[i][j] == board[i][k]) {
                    board[i][j] *= 2;
                    if (board[i][j] == 2048) win_flag = 1;
                    board[i][k] = 0;
                    moved = 1;
                }
                break;
            }
        }
        for (j = 0; j < SIZE; j++) {
            if (board[i][j] == 0) {
                for (k = j+1; k < SIZE; k++) {
                    if (board[i][k] != 0) {
                        board[i][j] = board[i][k];
                        board[i][k] = 0;
                        moved = 1;
                        break;
                    }
                }
            }
        }
    }
    return moved;
}

static uint8_t move_right(void)
{
    uint8_t moved = 0;
    int i, j, k;
    for (i = 0; i < SIZE; i++) {
        for (j = SIZE-1; j > 0; j--) {
            if (board[i][j] == 0) continue;
            for (k = j-1; k >= 0; k--) {
                if (board[i][k] == 0) continue;
                if (board[i][j] == board[i][k]) {
                    board[i][j] *= 2;
                    if (board[i][j] == 2048) win_flag = 1;
                    board[i][k] = 0;
                    moved = 1;
                }
                break;
            }
        }
        for (j = SIZE-1; j >= 0; j--) {
            if (board[i][j] == 0) {
                for (k = j-1; k >= 0; k--) {
                    if (board[i][k] != 0) {
                        board[i][j] = board[i][k];
                        board[i][k] = 0;
                        moved = 1;
                        break;
                    }
                }
            }
        }
    }
    return moved;
}

uint8_t Game_Move(uint8_t dir)
{
    uint8_t moved = 0;
    switch (dir) {
        case KEY_UP:    moved = move_up();    break;
        case KEY_DOWN:  moved = move_down();  break;
        case KEY_LEFT:  moved = move_left();  break;
        case KEY_RIGHT: moved = move_right(); break;
        default: break;
    }
    return moved;
}

uint8_t Game_IsWin(void)
{
    return win_flag;
}

uint8_t Game_IsLose(void)
{
    uint8_t i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (board[i][j] == 0) return 0;
            if (j+1 < SIZE && board[i][j] == board[i][j+1]) return 0;
            if (i+1 < SIZE && board[i][j] == board[i+1][j]) return 0;
        }
    }
    return 1;
}

void Game_ShowBoard(void)
{
    uint16_t w = LCD_WIDTH / SIZE;
    uint16_t h = LCD_HEIGHT / SIZE;
    uint8_t i, j;
    uint16_t color;
    int16_t x, y;   // 使用有符号避免警告
    uint16_t value;
    
    LCD_Fill(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1, BLACK);
    for (i = 0; i <= SIZE; i++) {
        LCD_Fill(0, i*h, LCD_WIDTH-1, i*h, BLACK);
        LCD_Fill(i*w, 0, i*w, LCD_HEIGHT-1, BLACK);
    }
    
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            x = j * w + 2;
            y = i * h + 2;
            value = board[i][j];
            if (value == 0) color = 0x4210;
            else if (value == 2) color = 0xFFFF;
            else if (value == 4) color = 0xFFE0;
            else if (value == 8) color = 0xFD20;
            else if (value == 16) color = 0xF800;
            else if (value == 32) color = 0x07E0;
            else if (value == 64) color = 0x001F;
            else if (value == 128) color = 0xF81F;
            else if (value == 256) color = 0xFFE0;
            else if (value == 512) color = 0x07FF;
            else if (value == 1024) color = 0xF81F;
            else if (value == 2048) color = 0xFFFF;
            else color = 0xFFFF;
            LCD_Fill(x, y, x + w - 5, y + h - 5, color);
        }
    }
}