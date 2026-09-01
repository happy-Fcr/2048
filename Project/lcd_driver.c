#include "lcd_driver.h"
#include "systick.h"
#include "gd32f4xx_spi.h"

// ---------- ?? SPI ----------
#define LCD_SPI_PERIPH SPI4

static void LCD_SPI_Write(uint8_t data)
{
    while (spi_i2s_flag_get(LCD_SPI_PERIPH, SPI_FLAG_TBE) == RESET) {
    }
    spi_i2s_data_transmit(LCD_SPI_PERIPH, data);
    while (spi_i2s_flag_get(LCD_SPI_PERIPH, SPI_FLAG_RBNE) == RESET) {
    }
    (void)spi_i2s_data_receive(LCD_SPI_PERIPH);
}

static void LCD_WriteCmd(uint8_t cmd)
{
    gpio_bit_reset(LCD_DC_PORT, LCD_DC_PIN);
    gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN);

    LCD_SPI_Write(cmd);

    gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN);
}

static void LCD_WriteData(uint8_t data)
{
    gpio_bit_set(LCD_DC_PORT, LCD_DC_PIN);
    gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN);

    LCD_SPI_Write(data);

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
    rcu_periph_clock_enable(RCU_SPI4);

    gpio_mode_set(LCD_RST_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_RST_PIN);
    gpio_output_options_set(LCD_RST_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_RST_PIN);

    gpio_mode_set(LCD_DC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_DC_PIN);
    gpio_output_options_set(LCD_DC_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_DC_PIN);

    gpio_mode_set(LCD_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_CS_PIN);
    gpio_output_options_set(LCD_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_CS_PIN);

    gpio_af_set(LCD_SCK_PORT, GPIO_AF_5, LCD_SCK_PIN | LCD_MOSI_PIN);
    gpio_mode_set(LCD_SCK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, LCD_SCK_PIN | LCD_MOSI_PIN);
    gpio_output_options_set(LCD_SCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_SCK_PIN | LCD_MOSI_PIN);

    gpio_mode_set(LCD_BL_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LCD_BL_PIN);
    gpio_output_options_set(LCD_BL_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LCD_BL_PIN);

    gpio_bit_set(LCD_BL_PORT, LCD_BL_PIN);
    gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN);

    {
        spi_parameter_struct spi_initpara;
        spi_i2s_deinit(LCD_SPI_PERIPH);
        spi_struct_para_init(&spi_initpara);
        spi_initpara.device_mode = SPI_MASTER;
        spi_initpara.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
        spi_initpara.frame_size = SPI_FRAMESIZE_8BIT;
        spi_initpara.nss = SPI_NSS_SOFT;
        spi_initpara.endian = SPI_ENDIAN_MSB;
        spi_initpara.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        spi_initpara.prescale = SPI_PSC_2;
        spi_init(LCD_SPI_PERIPH, &spi_initpara);
        spi_enable(LCD_SPI_PERIPH);
    }
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
    uint8_t i;

    uint8_t gamma_pos[14] = {
        0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F,
        0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23
    };

    uint8_t gamma_neg[14] = {
        0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F,
        0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23
    };

    LCD_WriteCmd(0x11);
    delay_1ms(120);

    LCD_WriteCmd(0x36);
    LCD_WriteData(0x00);

    LCD_WriteCmd(0x3A);
    LCD_WriteData(0x05);

    LCD_WriteCmd(0xB2);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x33);
    LCD_WriteData(0x33);

    LCD_WriteCmd(0xB7);
    LCD_WriteData(0x35);

    LCD_WriteCmd(0xBB);
    LCD_WriteData(0x28);

    LCD_WriteCmd(0xC0);
    LCD_WriteData(0x2C);

    LCD_WriteCmd(0xC2);
    LCD_WriteData(0x01);

    LCD_WriteCmd(0xC3);
    LCD_WriteData(0x0F);

    LCD_WriteCmd(0xC4);
    LCD_WriteData(0x20);

    LCD_WriteCmd(0xC6);
    LCD_WriteData(0x0F);

    LCD_WriteCmd(0xD0);
    LCD_WriteData(0xA4);
    LCD_WriteData(0xA1);

    LCD_WriteCmd(0xE0);
    for (i = 0; i < 14; i++) {
        LCD_WriteData(gamma_pos[i]);
    }

    LCD_WriteCmd(0xE1);
    for (i = 0; i < 14; i++) {
        LCD_WriteData(gamma_neg[i]);
    }

    LCD_WriteCmd(0x21);

    LCD_WriteCmd(0x29);
    delay_1ms(50);
}

void LCD_Init(void)
{
    LCD_GPIO_Init();
    LCD_Reset();
    LCD_InitReg();

    // ?????????,???????
    LCD_Clear(0x0000);
}

void LCD_Clear(uint16_t color)
{
    uint16_t i, j;

    LCD_WriteCmd(0x2A);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);
    LCD_WriteData(LCD_WIDTH - 1);

    LCD_WriteCmd(0x2B);
    LCD_WriteData(0x00);
    LCD_WriteData(20);
    LCD_WriteData(0x01);
    LCD_WriteData(0x2B);

    LCD_WriteCmd(0x2C);

    for (i = 0; i < LCD_HEIGHT; i++) {
        for (j = 0; j < LCD_WIDTH; j++) {
            LCD_WriteData16(color);
        }
    }
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return;
    }

    y += 20;

    LCD_WriteCmd(0x2A);
    LCD_WriteData(x >> 8);
    LCD_WriteData(x);
    LCD_WriteData(x >> 8);
    LCD_WriteData(x);

    LCD_WriteCmd(0x2B);
    LCD_WriteData(y >> 8);
    LCD_WriteData(y);
    LCD_WriteData(y >> 8);
    LCD_WriteData(y);

    LCD_WriteCmd(0x2C);
    LCD_WriteData16(color);
}

void LCD_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    y1 += 20;
    y2 += 20;

    LCD_WriteCmd(0x2A);
    LCD_WriteData(x1 >> 8);
    LCD_WriteData(x1 & 0xFF);
    LCD_WriteData(x2 >> 8);
    LCD_WriteData(x2 & 0xFF);

    LCD_WriteCmd(0x2B);
    LCD_WriteData(y1 >> 8);
    LCD_WriteData(y1 & 0xFF);
    LCD_WriteData(y2 >> 8);
    LCD_WriteData(y2 & 0xFF);

    LCD_WriteCmd(0x2C);
}

void LCD_Fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t total;
    uint32_t i;

    if (x1 >= LCD_WIDTH || y1 >= LCD_HEIGHT) return;
    if (x2 >= LCD_WIDTH) x2 = LCD_WIDTH - 1;
    if (y2 >= LCD_HEIGHT) y2 = LCD_HEIGHT - 1;

    LCD_SetWindow(x1, y1, x2, y2);

    total = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);

    gpio_bit_set(LCD_DC_PORT, LCD_DC_PIN);
    gpio_bit_reset(LCD_CS_PORT, LCD_CS_PIN);

    for (i = 0; i < total; i++) {
        LCD_SPI_Write(color >> 8);
        LCD_SPI_Write(color & 0xFF);
    }

    gpio_bit_set(LCD_CS_PORT, LCD_CS_PIN);
}

static const uint8_t font_5x7[96][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x14,0x08,0x3E,0x08,0x14}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x7F,0x41,0x41,0x00}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* backslash */
    {0x00,0x41,0x41,0x7F,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */
    {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */
    {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7F,0x48,0x44,0x44,0x38}, /* b */
    {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7F}, /* d */
    {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7E,0x09,0x01,0x02}, /* f */
    {0x0C,0x52,0x52,0x52,0x3E}, /* g */
    {0x7F,0x08,0x04,0x04,0x78}, /* h */
    {0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3D,0x00}, /* j */
    {0x7F,0x10,0x28,0x44,0x00}, /* k */
    {0x00,0x41,0x7F,0x40,0x00}, /* l */
    {0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x7C,0x08,0x04,0x04,0x78}, /* n */
    {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7C,0x14,0x14,0x14,0x08}, /* p */
    {0x08,0x14,0x14,0x18,0x7C}, /* q */
    {0x7C,0x08,0x04,0x04,0x08}, /* r */
    {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3F,0x44,0x40,0x20}, /* t */
    {0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C}, /* v */
    {0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */
    {0x0C,0x50,0x50,0x50,0x3C}, /* y */
    {0x44,0x64,0x54,0x4C,0x44}, /* z */
    {0x00,0x08,0x36,0x41,0x00}, /* { */
    {0x00,0x00,0x7F,0x00,0x00}, /* | */
    {0x00,0x41,0x36,0x08,0x00}, /* } */
    {0x10,0x08,0x08,0x10,0x08}, /* ~ */
    {0x00,0x00,0x00,0x00,0x00}
};

void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg)
{
    uint8_t col, row;
    uint8_t data;
    uint8_t index;
    uint8_t scale = 2;

    if ((uint8_t)ch < 32 || (uint8_t)ch > 127) {
        ch = ' ';
    }

    index = (uint8_t)ch - 32;
    LCD_Fill(x, y, x + 11, y + 15, bg);

    for (col = 0; col < 5; col++) {
        data = font_5x7[index][col];
        for (row = 0; row < 7; row++) {
            if (data & (1 << row)) {
                LCD_Fill(x + col * scale, y + row * scale,
                         x + col * scale + scale - 1,
                         y + row * scale + scale - 1, fg);
            }
        }
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, const char* str, uint16_t fg, uint16_t bg)
{
    while (*str) {
        LCD_ShowChar(x, y, *str, fg, bg);
        x += 12;
        str++;
    }
}

void LCD_ShowNum(uint16_t x, uint16_t y, uint16_t num, uint16_t fg, uint16_t bg)
{
    char buf[6];
    uint8_t i = 0;
    uint8_t j;
    char temp;

    if (num == 0) {
        LCD_ShowChar(x, y, '0', fg, bg);
        return;
    }

    while (num > 0) {
        buf[i++] = num % 10 + '0';
        num /= 10;
    }

    for (j = 0; j < i / 2; j++) {
        temp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = temp;
    }

    buf[i] = '\0';

    LCD_ShowString(x, y, buf, fg, bg);
}