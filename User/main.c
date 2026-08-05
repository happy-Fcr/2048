#include "gd32f4xx.h"
#include "systick.h"
#include "lcd_driver.h"
#include "game2048.h"
#include "key.h"
#include "rank_storage.h"
#include "sound.h"
#include "ui_language.h"
#include <stdlib.h>

#define UI_START      0
#define UI_RANK       1
#define UI_DIFFICULT  2
#define UI_GAME       3
#define UI_OVER       4
#define UI_WIN        5
#define UI_SETTING    6
#define UI_EXIT       7

static uint8_t ui_state = UI_START;
static uint8_t menu_index = 0;
static uint8_t diff_index = GAME_DIFF_NORMAL;
static uint8_t setting_index = 0;
static uint8_t volume_level = 3;
static uint8_t language = UI_LANG_ENGLISH;
static uint32_t rank_score[RANK_COUNT] = {0};
static uint8_t random_seeded = 0;

static void button_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOG);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_mode_set(GPIOG, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_9);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_15);
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_0);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_2);
}

static uint8_t read_key(void)
{
    if (gpio_input_bit_get(GPIOG, GPIO_PIN_9) == SET) {
        delay_1ms(20);
        if (gpio_input_bit_get(GPIOG, GPIO_PIN_9) == SET) return KEY_UP;
    }
    if (gpio_input_bit_get(GPIOB, GPIO_PIN_15) == SET) {
        delay_1ms(20);
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_15) == SET) return KEY_DOWN;
    }
    if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) == SET) {
        delay_1ms(20);
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) == SET) return KEY_LEFT;
    }
    if (gpio_input_bit_get(GPIOB, GPIO_PIN_2) == SET) {
        delay_1ms(20);
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_2) == SET) return KEY_RIGHT;
    }
    return KEY_NONE;
}

static void wait_key_release(void)
{
    while (read_key() != KEY_NONE);
    delay_1ms(50);
}

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

static uint8_t Rank_Add(uint32_t s)
{
    uint8_t i, j;

    if (s == 0) return 0;

    for (i = 0; i < RANK_COUNT; i++) {
        if (s > rank_score[i]) {
            for (j = RANK_COUNT - 1; j > i; j--) {
                rank_score[j] = rank_score[j - 1];
            }
            rank_score[i] = s;
            return 1;
        }
    }

    return 0;
}

static void Draw_StartMenu(void)
{
    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);
    LCD_ShowString(86, 30, "2048", WHITE, BLACK);
    UI_ShowStartItem(45, 75, 0, menu_index == 0, WHITE, BLACK);
    UI_ShowStartItem(45, 105, 1, menu_index == 1, WHITE, BLACK);
    UI_ShowStartItem(45, 135, 2, menu_index == 2, WHITE, BLACK);
    UI_ShowStartItem(45, 165, 3, menu_index == 3, WHITE, BLACK);
    UI_ShowStartItem(45, 195, 4, menu_index == 4, WHITE, BLACK);
    if (UI_GetLanguage() == UI_LANG_ENGLISH) {
        LCD_ShowString(20, 225, "UP/DOWN:SEL", WHITE, BLACK);
        LCD_ShowString(20, 245, "R:OK L:BACK", WHITE, BLACK);
    }
}

static const char *DiffName(uint8_t d)
{
    if (d == GAME_DIFF_EASY) return "EASY";
    if (d == GAME_DIFF_HARD) return "HARD";
    return "NORMAL";
}

static void Draw_Difficulty(void)
{
    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);
    if (UI_GetLanguage() == UI_LANG_CHINESE) {
        UI_ShowText(104, 28, UI_TEXT_DIFFICULTY, WHITE, BLACK);
        LCD_ShowString(35, 75, diff_index == GAME_DIFF_EASY ? ">" : " ", WHITE, BLACK);
        UI_ShowText(60, 75, UI_TEXT_EASY, WHITE, BLACK);
        LCD_ShowString(115, 75, "2:90%", WHITE, BLACK);
        LCD_ShowString(35, 105, diff_index == GAME_DIFF_NORMAL ? ">" : " ", WHITE, BLACK);
        UI_ShowText(60, 105, UI_TEXT_NORMAL, WHITE, BLACK);
        LCD_ShowString(115, 105, "2:80%", WHITE, BLACK);
        LCD_ShowString(35, 135, diff_index == GAME_DIFF_HARD ? ">" : " ", WHITE, BLACK);
        UI_ShowText(60, 135, UI_TEXT_HARD, WHITE, BLACK);
        LCD_ShowString(115, 135, "2:65%", WHITE, BLACK);
    } else {
        LCD_ShowString(50, 28, "DIFFICULTY", WHITE, BLACK);
        LCD_ShowString(35, 75,  diff_index == GAME_DIFF_EASY   ? "> EASY   2:90%" : "  EASY   2:90%", WHITE, BLACK);
        LCD_ShowString(35, 105, diff_index == GAME_DIFF_NORMAL ? "> NORMAL 2:80%" : "  NORMAL 2:80%", WHITE, BLACK);
        LCD_ShowString(35, 135, diff_index == GAME_DIFF_HARD   ? "> HARD   2:65%" : "  HARD   2:65%", WHITE, BLACK);
        LCD_ShowString(10, 200, "RIGHT OK  LEFT BACK", WHITE, BLACK);
    }
}

static void Draw_Rank(void)
{
    uint8_t i;
    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);
    if (UI_GetLanguage() == UI_LANG_CHINESE) {
        UI_ShowText(104, 12, UI_TEXT_RANK, WHITE, BLACK);
    } else {
        LCD_ShowString(70, 12, "TOP 10", WHITE, BLACK);
    }

    for (i = 0; i < RANK_COUNT; i++) {
        uint16_t y = 38 + i * 18;
        LCD_ShowNum(25, y, i + 1, WHITE, BLACK);
        LCD_ShowString(45, y, ".", WHITE, BLACK);
        LCD_ShowUInt32(70, y, rank_score[i], WHITE, BLACK);
    }

    if (UI_GetLanguage() == UI_LANG_ENGLISH) {
        LCD_ShowString(10, 220, "LEFT BACK", WHITE, BLACK);
    }
}

static void Draw_End(uint8_t is_win)
{
    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);

    if (UI_GetLanguage() == UI_LANG_CHINESE) {
        UI_ShowText(104, 45, is_win ? UI_TEXT_WIN : UI_TEXT_GAME_OVER, WHITE, BLACK);
        UI_ShowText(55, 90, UI_TEXT_SCORE, WHITE, BLACK);
        LCD_ShowUInt32(100, 90, Game_GetScore(), WHITE, BLACK);
        UI_ShowText(55, 125, UI_TEXT_DIFFICULTY, WHITE, BLACK);
        UI_ShowText(110, 125, (uint8_t)(UI_TEXT_EASY + Game_GetDifficulty()), WHITE, BLACK);
    } else if (is_win) {
        LCD_ShowString(78, 45, "YOU WIN", WHITE, BLACK);
    } else {
        LCD_ShowString(70, 45, "GAME OVER", WHITE, BLACK);
    }

    if (UI_GetLanguage() == UI_LANG_ENGLISH) {
        LCD_ShowString(45, 90, "SCORE:", WHITE, BLACK);
        LCD_ShowUInt32(125, 90, Game_GetScore(), WHITE, BLACK);
        LCD_ShowString(45, 125, "DIFF:", WHITE, BLACK);
        LCD_ShowString(105, 125, DiffName(Game_GetDifficulty()), WHITE, BLACK);
        LCD_ShowString(18, 190, "RIGHT RESTART", WHITE, BLACK);
        LCD_ShowString(18, 215, "LEFT MENU", WHITE, BLACK);
    }
}

static void Start_NewGame(void)
{
    if (!random_seeded) {
        /* The first key press supplies sub-millisecond timing entropy. */
        srand((uint32_t)SysTick->VAL ^
              ((uint32_t)gpio_input_port_get(GPIOA) << 16) ^
              gpio_input_port_get(GPIOB) ^ gpio_input_port_get(GPIOG));
        random_seeded = 1;
    }

    Game_SetDifficulty(diff_index);
    Game_Init();
    Game_ShowBoard();
    Sound_StartMusic();
    ui_state = UI_GAME;
}

static void Draw_Settings(void)
{
    char volume_text[8] = "< 0 >";

    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);
    UI_ShowText(72, 28, UI_TEXT_SETTINGS, WHITE, BLACK);

    LCD_ShowString(25, 82, setting_index == 0 ? ">" : " ", WHITE, BLACK);
    UI_ShowText(50, 82, UI_TEXT_VOLUME, WHITE, BLACK);
    volume_text[2] = (char)('0' + volume_level);
    LCD_ShowString(145, 82, volume_text, WHITE, BLACK);

    LCD_ShowString(25, 122, setting_index == 1 ? ">" : " ", WHITE, BLACK);
    UI_ShowText(50, 122, UI_TEXT_LANGUAGE, WHITE, BLACK);
    if (language == UI_LANG_CHINESE) {
        UI_ShowText(155, 122, UI_TEXT_CHINESE, WHITE, BLACK);
    } else {
        UI_ShowText(145, 122, UI_TEXT_ENGLISH, WHITE, BLACK);
    }

    LCD_ShowString(25, 162, setting_index == 2 ? ">" : " ", WHITE, BLACK);
    UI_ShowText(50, 162, UI_TEXT_EXIT, WHITE, BLACK);

    if (UI_GetLanguage() == UI_LANG_ENGLISH) {
        LCD_ShowString(20, 205, "UP/DOWN:SEL", WHITE, BLACK);
        LCD_ShowString(20, 225, "L/R:ADJUST", WHITE, BLACK);
        LCD_ShowString(20, 245, "SELECT EXIT", WHITE, BLACK);
    }
}

static void Draw_Exit(void)
{
    LCD_Fill(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK);
    UI_ShowText(104, 115, UI_TEXT_EXIT, WHITE, BLACK);
    if (UI_GetLanguage() == UI_LANG_ENGLISH) {
        LCD_ShowString(48, 170, "LEFT/RIGHT: MENU", WHITE, BLACK);
    }
}

int main(void)
{
    uint8_t key;

    systick_config();
    LCD_Init();
    button_init();
    Sound_Init();
    Sound_SetVolume(volume_level);
    RankStorage_Load(rank_score);

    Draw_StartMenu();

    while (1) {
        key = read_key();
        Sound_Task();

        if (key == KEY_NONE) continue;

        if (ui_state == UI_START) {
            if (key == KEY_UP) {
                menu_index = (menu_index == 0) ? 4 : menu_index - 1;
                Draw_StartMenu();
            } else if (key == KEY_DOWN) {
                menu_index = (menu_index + 1) % 5;
                Draw_StartMenu();
            } else if (key == KEY_RIGHT) {
                if (menu_index == 0) {
                    Start_NewGame();
                } else if (menu_index == 1) {
                    ui_state = UI_RANK;
                    Draw_Rank();
                } else if (menu_index == 2) {
                    ui_state = UI_DIFFICULT;
                    Draw_Difficulty();
                } else if (menu_index == 3) {
                    ui_state = UI_SETTING;
                    Draw_Settings();
                } else {
                    Sound_Stop();
                    ui_state = UI_EXIT;
                    Draw_Exit();
                }
            }
        } else if (ui_state == UI_RANK) {
            if (key == KEY_LEFT) {
                ui_state = UI_START;
                Draw_StartMenu();
            }
        } else if (ui_state == UI_DIFFICULT) {
            if (key == KEY_UP) {
                diff_index = (diff_index == GAME_DIFF_EASY) ? GAME_DIFF_HARD : diff_index - 1;
                Draw_Difficulty();
            } else if (key == KEY_DOWN) {
                diff_index = (diff_index + 1) % 3;
                Draw_Difficulty();
            } else if (key == KEY_RIGHT) {
                Game_SetDifficulty(diff_index);
                ui_state = UI_START;
                Draw_StartMenu();
            } else if (key == KEY_LEFT) {
                ui_state = UI_START;
                Draw_StartMenu();
            }
        } else if (ui_state == UI_SETTING) {
            if (key == KEY_UP || key == KEY_DOWN) {
                if (key == KEY_UP) {
                    setting_index = (setting_index == 0) ? 2 : setting_index - 1;
                } else {
                    setting_index = (setting_index + 1) % 3;
                }
                Draw_Settings();
            } else if (key == KEY_LEFT) {
                if (setting_index == 0 && volume_level > 0) {
                    volume_level--;
                    Sound_SetVolume(volume_level);
                    Draw_Settings();
                } else if (setting_index == 1) {
                    language = UI_LANG_ENGLISH;
                    UI_SetLanguage(language);
                    Draw_Settings();
                } else if (setting_index == 2) {
                    ui_state = UI_START;
                    Draw_StartMenu();
                }
            } else if (key == KEY_RIGHT) {
                if (setting_index == 0 && volume_level < 5) {
                    volume_level++;
                    Sound_SetVolume(volume_level);
                    Draw_Settings();
                } else if (setting_index == 1) {
                    language = UI_LANG_CHINESE;
                    UI_SetLanguage(language);
                    Draw_Settings();
                } else if (setting_index == 2) {
                    ui_state = UI_START;
                    Draw_StartMenu();
                }
            }
        } else if (ui_state == UI_EXIT) {
            if (key == KEY_LEFT || key == KEY_RIGHT) {
                ui_state = UI_START;
                Draw_StartMenu();
            }
        } else if (ui_state == UI_GAME) {
            if (key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT) {
                if (Game_Move(key)) {
                    Game_ShowBoard();
                    if (Game_IsWin()) {
                        if (Rank_Add(Game_GetScore())) {
                            RankStorage_Save(rank_score);
                        }
                        Sound_PlayWin();
                        ui_state = UI_WIN;
                        Draw_End(1);
                    } else if (Game_IsLose()) {
                        if (Rank_Add(Game_GetScore())) {
                            RankStorage_Save(rank_score);
                        }
                        Sound_PlayGameOver();
                        ui_state = UI_OVER;
                        Draw_End(0);
                    }
                }
            }
        } else if (ui_state == UI_OVER || ui_state == UI_WIN) {
            if (key == KEY_RIGHT) {
                Start_NewGame();
            } else if (key == KEY_LEFT) {
                ui_state = UI_START;
                Draw_StartMenu();
            }
        }

        wait_key_release();
    }
}
