#include "game2048.h"
#include "lcd_driver.h"
#include "key.h"
#include "ui_language.h"
#include <stdlib.h>

static uint16_t board[SIZE][SIZE];
static uint8_t win_flag = 0;
static uint8_t game_diff = GAME_DIFF_NORMAL;
static uint32_t score = 0;

static void LCD_ShowUInt32(uint16_t x, uint16_t y, uint32_t num, uint16_t fg, uint16_t bg)
{
    char buf[11];
    uint8_t i = 0, j;

    if (num == 0) {
        LCD_ShowChar(x, y, '0', fg, bg);
        return;
    }

    while (num > 0 && i < sizeof(buf)) {
        buf[i++] = (char)('0' + num % 10);
        num /= 10;
    }

    for (j = 0; j < i; j++) {
        LCD_ShowChar(x + j * 12, y, buf[i - 1 - j], fg, bg);
    }
}

void Game_SetDifficulty(uint8_t diff)
{
    if (diff > GAME_DIFF_HARD) diff = GAME_DIFF_NORMAL;
    game_diff = diff;
}

uint8_t Game_GetDifficulty(void)
{
    return game_diff;
}

uint32_t Game_GetScore(void)
{
    return score;
}

void Game_Init(void)
{
    uint8_t i, j;

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            board[i][j] = 0;
        }
    }

    win_flag = 0;
    score = 0;

    Game_GenerateNumber();
    Game_GenerateNumber();
}

void Game_GenerateNumber(void)
{
    uint8_t empty_cells[SIZE * SIZE][2];
    uint8_t empty_count = 0;
    uint8_t i, j, pos;
    uint16_t value;
    uint8_t r;

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
    r = rand() % 100;

    /* EASY: 2?90%, NORMAL: 2?80%, HARD: 2?65% */
    if (game_diff == GAME_DIFF_EASY) {
        value = (r < 90) ? 2 : 4;
    } else if (game_diff == GAME_DIFF_HARD) {
        value = (r < 65) ? 2 : 4;
    } else {
        value = (r < 80) ? 2 : 4;
    }

    board[empty_cells[pos][0]][empty_cells[pos][1]] = value;
}

static uint8_t move_up(void)
{
    uint8_t moved = 0;
    int i, j, k;

    for (j = 0; j < SIZE; j++) {
        for (i = 0; i < SIZE - 1; i++) {
            if (board[i][j] == 0) continue;

            for (k = i + 1; k < SIZE; k++) {
                if (board[k][j] == 0) continue;

                if (board[i][j] == board[k][j]) {
                    board[i][j] *= 2;
                    score += board[i][j];
                    if (board[i][j] == 2048) win_flag = 1;
                    board[k][j] = 0;
                    moved = 1;
                }
                break;
            }
        }

        for (i = 0; i < SIZE; i++) {
            if (board[i][j] == 0) {
                for (k = i + 1; k < SIZE; k++) {
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
        for (i = SIZE - 1; i > 0; i--) {
            if (board[i][j] == 0) continue;

            for (k = i - 1; k >= 0; k--) {
                if (board[k][j] == 0) continue;

                if (board[i][j] == board[k][j]) {
                    board[i][j] *= 2;
                    score += board[i][j];
                    if (board[i][j] == 2048) win_flag = 1;
                    board[k][j] = 0;
                    moved = 1;
                }
                break;
            }
        }

        for (i = SIZE - 1; i >= 0; i--) {
            if (board[i][j] == 0) {
                for (k = i - 1; k >= 0; k--) {
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
        for (j = 0; j < SIZE - 1; j++) {
            if (board[i][j] == 0) continue;

            for (k = j + 1; k < SIZE; k++) {
                if (board[i][k] == 0) continue;

                if (board[i][j] == board[i][k]) {
                    board[i][j] *= 2;
                    score += board[i][j];
                    if (board[i][j] == 2048) win_flag = 1;
                    board[i][k] = 0;
                    moved = 1;
                }
                break;
            }
        }

        for (j = 0; j < SIZE; j++) {
            if (board[i][j] == 0) {
                for (k = j + 1; k < SIZE; k++) {
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
        for (j = SIZE - 1; j > 0; j--) {
            if (board[i][j] == 0) continue;

            for (k = j - 1; k >= 0; k--) {
                if (board[i][k] == 0) continue;

                if (board[i][j] == board[i][k]) {
                    board[i][j] *= 2;
                    score += board[i][j];
                    if (board[i][j] == 2048) win_flag = 1;
                    board[i][k] = 0;
                    moved = 1;
                }
                break;
            }
        }

        for (j = SIZE - 1; j >= 0; j--) {
            if (board[i][j] == 0) {
                for (k = j - 1; k >= 0; k--) {
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
        case KEY_UP:
            moved = move_up();
            break;
        case KEY_DOWN:
            moved = move_down();
            break;
        case KEY_LEFT:
            moved = move_left();
            break;
        case KEY_RIGHT:
            moved = move_right();
            break;
        default:
            break;
    }

    if (moved) {
        Game_GenerateNumber();
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
            if (j + 1 < SIZE && board[i][j] == board[i][j + 1]) return 0;
            if (i + 1 < SIZE && board[i][j] == board[i + 1][j]) return 0;
        }
    }

    return 1;
}

static uint16_t GetBlockColor(uint16_t value)
{
    switch (value) {
        case 0:    return 0x4210;
        case 2:    return 0xFEBA;
        case 4:    return 0xFD20;
        case 8:    return 0xFBE0;
        case 16:   return 0xFFE0;
        case 32:   return 0x07E0;
        case 64:   return 0x07FF;
        case 128:  return 0x041F;
        case 256:  return 0x001F;
        case 512:  return 0xF81F;
        case 1024: return 0xF81F;
        case 2048: return 0xFEA0;
        default:   return 0xFFFF;
    }
}

static void DrawBlockNum(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t value, uint16_t bg)
{
    uint16_t num_x;
    uint16_t num_y;

    if (value == 0) return;

    if (value < 10) {
        num_x = x + w / 2 - 6;
    } else if (value < 100) {
        num_x = x + w / 2 - 12;
    } else if (value < 1000) {
        num_x = x + w / 2 - 18;
    } else {
        num_x = x + w / 2 - 24;
    }

    num_y = y + h / 2 - 8;
    LCD_ShowNum(num_x, num_y, value, WHITE, bg);
}

void Game_ShowBoard(void)
{
    uint16_t w = LCD_WIDTH / SIZE;
    uint16_t h = (LCD_HEIGHT - 24) / SIZE;
    uint8_t i, j;
    uint16_t color;
    uint16_t x, y;
    uint16_t value;

    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);
    if (UI_GetLanguage() == UI_LANG_CHINESE) {
        UI_ShowText(4, 4, UI_TEXT_SCORE, WHITE, BLACK);
        LCD_ShowUInt32(42, 4, score, WHITE, BLACK);
    } else {
        LCD_ShowString(4, 4, "SCORE:", WHITE, BLACK);
        LCD_ShowUInt32(76, 4, score, WHITE, BLACK);
    }

    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            x = j * w + 2;
            y = 24 + i * h + 2;
            value = board[i][j];
            color = GetBlockColor(value);
            LCD_Fill(x, y, x + w - 5, y + h - 5, color);
            DrawBlockNum(x, y, w - 5, h - 5, value, color);
        }
    }
}
