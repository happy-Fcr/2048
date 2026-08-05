#ifndef __UI_LANGUAGE_H
#define __UI_LANGUAGE_H

#include <stdint.h>

#define UI_LANG_ENGLISH 0U
#define UI_LANG_CHINESE 1U

#define UI_TEXT_SETTINGS  0U
#define UI_TEXT_VOLUME    1U
#define UI_TEXT_LANGUAGE  2U
#define UI_TEXT_CHINESE   3U
#define UI_TEXT_ENGLISH   4U
#define UI_TEXT_EXIT      5U
#define UI_TEXT_EASY      6U
#define UI_TEXT_NORMAL    7U
#define UI_TEXT_HARD      8U
#define UI_TEXT_RANK      9U
#define UI_TEXT_WIN       10U
#define UI_TEXT_GAME_OVER 11U
#define UI_TEXT_SCORE     12U
#define UI_TEXT_DIFFICULTY 13U

void UI_SetLanguage(uint8_t language);
uint8_t UI_GetLanguage(void);
void UI_ShowStartItem(uint16_t x, uint16_t y, uint8_t item, uint8_t selected,
                      uint16_t fg, uint16_t bg);
void UI_ShowText(uint16_t x, uint16_t y, uint8_t text_id, uint16_t fg, uint16_t bg);

#endif /* __UI_LANGUAGE_H */
