#include "lcd_driver.h"
#include "systick.h"

// ---------- 软件 SPI ----------
#define LCD_SCK_SET()  gpio_bit_set(LCD_SCK_PORT, LCD_SCK_PIN)
#define LCD_SCK_CLR()  gpio_bit_reset(LCD_SCK_PORT, LCD_SCK_PIN)
#define LCD_MOSI_SET() gpio_bit_set(LCD_MOSI_PORT, LCD_MOSI_PIN)
#define LCD_MOSI_CLR() gpio_bit_reset(LCD_MOSI_PORT, LCD_MOSI_PIN)

static void SW_SPI_Write(uint8_t dat)
{
    uint8_t i;
    volatile uint32_t dummy;
    for (i = 0; i < 8; i++) {
        if (dat & 0x80) LCD_MOSI_SET();
        else            LCD_MOSI_CLR();
        LCD_SCK_SET();
        for (dummy = 0; dummy < 2; dummy++);
        LCD_SCK_CLR();
        for (dummy = 0; dummy < 2; dummy++);
        dat <<= 1;
    }
}

static void LCD_WriteCmd(uint8_t cmd)
{
    gpio_bit_reset(LCD_DC_PORT, LCD_DC_PIN);
    gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN);
    SW_SPI_Write(cmd);
    gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN);
}

static void LCD_WriteData(uint8_t data)
{
    gpio_bit_set(LCD_DC_PORT, LCD_DC_PIN);
    gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN);
    SW_SPI_Write(data);
    gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN);
}

static void LCD_WriteData16(uint16_t data)
{
    LCD_WriteData(data >> 8);
    LCD_WriteData(data & 0xFF);
}

static void LCD_GPIO_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOF);
    rcu_periph_clock_enable(RCU_GPIOC);

    gpio_mode_set(LCD_RST_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_RST_PIN);
    gpio_output_options_set(LCD_RST_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_RST_PIN);
    gpio_mode_set(LCD_DC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_DC_PIN);
    gpio_output_options_set(LCD_DC_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_DC_PIN);
    gpio_mode_set(LCD_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_CS_PIN);
    gpio_output_options_set(LCD_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_CS_PIN);

    gpio_mode_set(LCD_SCK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_SCK_PIN);
    gpio_output_options_set(LCD_SCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_SCK_PIN);
    gpio_mode_set(LCD_MOSI_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_MOSI_PIN);
    gpio_output_options_set(LCD_MOSI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_MOSI_PIN);

    gpio_mode_set(LCD_BL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_BL_PIN);
    gpio_output_options_set(LCD_BL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_BL_PIN);
    gpio_bit_set(LCD_BL_PORT, LCD_BL_PIN);

    gpio_bit_reset(LCD_SCK_PORT, LCD_SCK_PIN);
    gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN);
}

static void LCD_Reset(void)
{
    gpio_bit_reset(LCD_RST_PORT, LCD_RST_PIN);
    delay_1ms(50);
    gpio_bit_set(LCD_RST_PORT, LCD_RST_PIN);
    delay_1ms(150);
}

static void LCD_InitReg(void)
{
    uint8_t gamma_pos[14] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
    uint8_t gamma_neg[14] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
    uint8_t i;

    LCD_WriteCmd(0x11); delay_1ms(120);
    LCD_WriteCmd(0x36); LCD_WriteData(0x00);
    LCD_WriteCmd(0x3A); LCD_WriteData(0x05);
    LCD_WriteCmd(0xB2); LCD_WriteData(0x0C); LCD_WriteData(0x0C); LCD_WriteData(0x00); LCD_WriteData(0x33); LCD_WriteData(0x33);
    LCD_WriteCmd(0xB7); LCD_WriteData(0x35);
    LCD_WriteCmd(0xBB); LCD_WriteData(0x28);
    LCD_WriteCmd(0xC0); LCD_WriteData(0x2C);
    LCD_WriteCmd(0xC2); LCD_WriteData(0x01);
    LCD_WriteCmd(0xC3); LCD_WriteData(0x0F);
    LCD_WriteCmd(0xC4); LCD_WriteData(0x20);
    LCD_WriteCmd(0xC6); LCD_WriteData(0x0F);
    LCD_WriteCmd(0xD0); LCD_WriteData(0xA4); LCD_WriteData(0xA1);

    LCD_WriteCmd(0xE0);
    for (i = 0; i < 14; i++) LCD_WriteData(gamma_pos[i]);
    LCD_WriteCmd(0xE1);
    for (i = 0; i < 14; i++) LCD_WriteData(gamma_neg[i]);

    LCD_WriteCmd(0x21);
    LCD_WriteCmd(0x29);
    delay_1ms(50);
}

void LCD_Init(void)
{
    LCD_GPIO_Init();

    LCD_Reset();

    LCD_InitReg();

    LCD_Clear(RED);   // ?关键：不是 BLACK，改红色

    while(1); // 卡死在这里
}
void LCD_Clear(uint16_t color)
{
    uint16_t i, j;
    LCD_WriteCmd(0x2A);  // 列地址
    LCD_WriteData(0x00); LCD_WriteData(0x00);
    LCD_WriteData(0x00); LCD_WriteData(LCD_WIDTH-1);
    LCD_WriteCmd(0x2B);  // 行地址：偏移20
    LCD_WriteData(0x00); LCD_WriteData(20);
    LCD_WriteData(0x01); LCD_WriteData(0x2B);
    LCD_WriteCmd(0x2C);
    for (i = 0; i < LCD_HEIGHT; i++) {
        for (j = 0; j < LCD_WIDTH; j++) {
            LCD_WriteData16(color);
        }
    }
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    // 应用行偏移 20
    y += 20;
    LCD_WriteCmd(0x2A);
    LCD_WriteData(x >> 8); LCD_WriteData(x);
    LCD_WriteData(x >> 8); LCD_WriteData(x);
    LCD_WriteCmd(0x2B);
    LCD_WriteData(y >> 8); LCD_WriteData(y);
    LCD_WriteData(y >> 8); LCD_WriteData(y);
    LCD_WriteCmd(0x2C);
    LCD_WriteData16(color);
}

void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t i, j;
    for (i = y1; i <= y2; i++) {
        for (j = x1; j <= x2; j++) {
            LCD_DrawPoint(j, i, color);
        }
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg)
{
    while(*str)
    {
        LCD_ShowChar(x, y, *str, fg, bg);
        x += 8;
        str++;
    }
}



void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg)
{
    // 临时版本：直接显示一个方块代替字符
    LCD_Fill(x, y, x+8, y+16, fg);
}