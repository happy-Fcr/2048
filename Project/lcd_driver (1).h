#ifndef __LCD_DRIVER_H
#define __LCD_DRIVER_H

#include "gd32f4xx.h"

// ========== 以下引脚配置根据你提供的信息一次性写好 ==========
#define LCD_RST_PORT    GPIOF
#define LCD_RST_PIN     GPIO_PIN_10

#define LCD_DC_PORT     GPIOA
#define LCD_DC_PIN      GPIO_PIN_6

#define LCD_CS_PORT     GPIOA
#define LCD_CS_PIN      GPIO_PIN_4

#define LCD_BL_PORT     GPIOA
#define LCD_BL_PIN      GPIO_PIN_7

// SPI 引脚（软件模拟，无需硬件 SPI）
#define LCD_SCK_PORT    GPIOF
#define LCD_SCK_PIN     GPIO_PIN_7
#define LCD_MOSI_PORT   GPIOF
#define LCD_MOSI_PIN    GPIO_PIN_9
// ======================================================

#define LCD_WIDTH       240
#define LCD_HEIGHT      280

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define GRAY    0x8430
#define YELLOW  0xFFE0
void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg);
void LCD_ShowString(uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg);
void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fg, uint16_t bg);

#endif