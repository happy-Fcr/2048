#include "game2048.h"
#include "lcd_driver.h"
#include "key.h"
#include "ui_language.h"
#include <stdlib.h>

static uint16_t board[SIZE][SIZE];
static uint16_t rendered_board[SIZE][SIZE];
static uint8_t render_valid = 0U;
static uint8_t win_flag = 0U;
static uint8_t game_diff = GAME_DIFF_NORMAL;
static uint32_t score = 0U;

static void LCD_ShowUInt32(uint16_t x, uint16_t y, uint32_t num, uint16_t fg, uint16_t bg)
{
    char buf[11];
    uint8_t i = 0U;
    uint8_t j;

    if (num == 0U) {
        LCD_ShowChar(x, y, '0', fg, bg);
        return;
    }

    while (num > 0U && i < sizeof(buf)) {
        buf[i++] = (char)('0' + num % 10U);
        num /= 10U;
    }

    for (j = 0U; j < i; j++) {
        LCD_ShowChar((uint16_t)(x + j * 12U), y, buf[i - 1U - j], fg, bg);
    }
}

void Game_SetDifficulty(uint8_t diff)
{
    game_diff = (diff <= GAME_DIFF_HARD) ? diff : GAME_DIFF_NORMAL;
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
    uint8_t row;
    uint8_t col;

    for (row = 0U; row < SIZE; row++) {
        for (col = 0U; col < SIZE; col++) {
            board[row][col] = 0U;
            rendered_board[row][col] = 0U;
        }
    }

    win_flag = 0U;
    score = 0U;
    render_valid = 0U;
    Game_GenerateNumber();
    Game_GenerateNumber();
}

void Game_GenerateNumber(void)
{
    uint8_t empty_cells[SIZE * SIZE][2];
    uint8_t empty_count = 0U;
    uint8_t row;
    uint8_t col;
    uint8_t pos;
    uint8_t random_percent;
    uint16_t value;

    for (row = 0U; row < SIZE; row++) {
        for (col = 0U; col < SIZE; col++) {
            if (board[row][col] == 0U) {
                empty_cells[empty_count][0] = row;
                empty_cells[empty_count][1] = col;
                empty_count++;
            }
        }
    }

    if (empty_count == 0U) return;

    pos = (uint8_t)(rand() % empty_count);
    random_percent = (uint8_t)(rand() % 100U);
    if (game_diff == GAME_DIFF_EASY) {
        value = (random_percent < 90U) ? 2U : 4U;
    } else if (game_diff == GAME_DIFF_HARD) {
        value = (random_percent < 65U) ? 2U : 4U;
    } else {
        value = (random_percent < 80U) ? 2U : 4U;
    }

    board[empty_cells[pos][0]][empty_cells[pos][1]] = value;
}

static uint8_t Game_MoveWithCore(uint8_t dir)
{
    uint16_t line[SIZE];
    uint8_t line_index;
    uint8_t cell_index;
    uint8_t moved = 0U;
    uint8_t reached_2048;
    uint32_t score_delta;

    for (line_index = 0U; line_index < SIZE; line_index++) {
        for (cell_index = 0U; cell_index < SIZE; cell_index++) {
            if (dir == KEY_UP) line[cell_index] = board[cell_index][line_index];
            else if (dir == KEY_DOWN) line[cell_index] = board[SIZE - 1U - cell_index][line_index];
            else if (dir == KEY_LEFT) line[cell_index] = board[line_index][cell_index];
            else line[cell_index] = board[line_index][SIZE - 1U - cell_index];
        }

        if (Game2048_ProcessLine(line, &score_delta, &reached_2048) != 0U) {
            moved = 1U;
        }
        score += score_delta;
        if (reached_2048 != 0U) win_flag = 1U;

        for (cell_index = 0U; cell_index < SIZE; cell_index++) {
            if (dir == KEY_UP) board[cell_index][line_index] = line[cell_index];
            else if (dir == KEY_DOWN) board[SIZE - 1U - cell_index][line_index] = line[cell_index];
            else if (dir == KEY_LEFT) board[line_index][cell_index] = line[cell_index];
            else board[line_index][SIZE - 1U - cell_index] = line[cell_index];
        }
    }

    return moved;
}

uint8_t Game_Move(uint8_t dir)
{
    uint8_t moved = 0U;

    if (dir == KEY_UP || dir == KEY_DOWN || dir == KEY_LEFT || dir == KEY_RIGHT) {
        moved = Game_MoveWithCore(dir);
    }

    if (moved != 0U) Game_GenerateNumber();
    return moved;
}

uint8_t Game_IsWin(void)
{
    return win_flag;
}

void Game_ContinueAfterWin(void)
{
    win_flag = 0U;
    render_valid = 0U;
}

uint8_t Game_IsLose(void)
{
    uint8_t row;
    uint8_t col;

    for (row = 0U; row < SIZE; row++) {
        for (col = 0U; col < SIZE; col++) {
            if (board[row][col] == 0U) return 0U;
            if (col + 1U < SIZE && board[row][col] == board[row][col + 1U]) return 0U;
            if (row + 1U < SIZE && board[row][col] == board[row + 1U][col]) return 0U;
        }
    }

    return 1U;
}

static uint16_t GetBlockColor(uint16_t value)
{
    switch (value) {
        case 0U:    return 0x4210U;
        case 2U:    return 0xFEBAU;
        case 4U:    return 0xFD20U;
        case 8U:    return 0xFBE0U;
        case 16U:   return 0xFFE0U;
        case 32U:   return 0x07E0U;
        case 64U:   return 0x07FFU;
        case 128U:  return 0x041FU;
        case 256U:  return 0x001FU;
        case 512U:  return 0xF81FU;
        case 1024U: return 0xF81FU;
        case 2048U: return 0xFEA0U;
        default:     return WHITE;
    }
}

static void DrawBlockNum(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         uint16_t value, uint16_t bg)
{
    uint16_t num_x;
    uint16_t num_y;

    if (value == 0U) return;

    if (value < 10U) num_x = (uint16_t)(x + w / 2U - 6U);
    else if (value < 100U) num_x = (uint16_t)(x + w / 2U - 12U);
    else if (value < 1000U) num_x = (uint16_t)(x + w / 2U - 18U);
    else num_x = (uint16_t)(x + w / 2U - 24U);

    num_y = (uint16_t)(y + h / 2U - 8U);
    LCD_ShowNum(num_x, num_y, value, WHITE, bg);
}

static void Game_DrawBlock(uint8_t row, uint8_t col, uint16_t value)
{
    uint16_t w = LCD_WIDTH / SIZE;
    uint16_t h = (LCD_HEIGHT - 24U) / SIZE;
    uint16_t x = (uint16_t)(col * w + 2U);
    uint16_t y = (uint16_t)(24U + row * h + 2U);
    uint16_t color = GetBlockColor(value);

    LCD_Fill(x, y, (uint16_t)(x + w - 5U), (uint16_t)(y + h - 5U), color);
    DrawBlockNum(x, y, (uint16_t)(w - 5U), (uint16_t)(h - 5U), value, color);
}

void Game_ShowBoard(void)
{
    uint8_t row;
    uint8_t col;
    uint8_t full_redraw = (render_valid == 0U);

    if (full_redraw != 0U) {
        LCD_Fill(0U, 0U, LCD_WIDTH - 1U, LCD_HEIGHT - 1U, BLACK);
    } else {
        LCD_Fill(40U, 0U, 180U, 20U, BLACK);
    }

    if (UI_GetLanguage() == UI_LANG_CHINESE) {
        UI_ShowText(4U, 4U, UI_TEXT_SCORE, WHITE, BLACK);
        LCD_ShowUInt32(42U, 4U, score, WHITE, BLACK);
    } else {
        LCD_ShowString(4U, 4U, "SCORE:", WHITE, BLACK);
        LCD_ShowUInt32(76U, 4U, score, WHITE, BLACK);
    }

    /* First erase every changed cell, then draw its final value.  This keeps
     * old digits from surviving after a move or merge without full-screen redraws. */
    for (row = 0U; row < SIZE; row++) {
        for (col = 0U; col < SIZE; col++) {
            if (full_redraw != 0U || rendered_board[row][col] != board[row][col]) {
                Game_DrawBlock(row, col, 0U);
            }
        }
    }

    for (row = 0U; row < SIZE; row++) {
        for (col = 0U; col < SIZE; col++) {
            if (full_redraw != 0U || rendered_board[row][col] != board[row][col]) {
                Game_DrawBlock(row, col, board[row][col]);
                rendered_board[row][col] = board[row][col];
            }
        }
    }

    render_valid = 1U;
}
