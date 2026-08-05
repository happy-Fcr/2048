#ifndef __LCD_H
#define __LCD_H
#include "gd32f4xx.h"

#define LCD_WIDTH  240
#define LCD_HEIGHT 280

// 常用颜色 (RGB565)
#define WHITE       0xFFFF
#define BLACK       0x0000
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define YELLOW      0xFFE0
#define GRAY        0x8410

// 引脚控制
#define LCD_RST(x)  gpio_bit_write(GPIOF, GPIO_PIN_10, (x) ? SET : RESET)
#define LCD_DC(x)   gpio_bit_write(GPIOA, GPIO_PIN_6,  (x) ? SET : RESET)
#define LCD_CS(x)   gpio_bit_write(GPIOA, GPIO_PIN_4,  (x) ? SET : RESET)
#define LCD_BLK(x)  gpio_bit_write(GPIOA, GPIO_PIN_7,  (x) ? SET : RESET)

void lcd_init(void);
void lcd_set_region(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
#endif