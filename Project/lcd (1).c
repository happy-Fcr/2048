#include "lcd.h"

/* 引用 main.c 里的精准毫秒延时（由 DWT 实现） */
extern void delay_ms(uint32_t ms);

/* ---------- 底层 SPI 写入 ---------- */
static void spi_write_byte(uint8_t data) {
    while(RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE));
    spi_i2s_data_transmit(SPI2, data);
    while(RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE));
    spi_i2s_data_receive(SPI2);                         // 清 RXNE
}

static void lcd_write_cmd(uint8_t cmd) {
    LCD_DC(0);
    LCD_CS(0);
    spi_write_byte(cmd);
    LCD_CS(1);
}

static void lcd_write_data(uint8_t data) {
    LCD_DC(1);
    LCD_CS(0);
    spi_write_byte(data);
    LCD_CS(1);
}

static void lcd_write_data16(uint16_t data) {
    LCD_DC(1);
    LCD_CS(0);
    spi_write_byte(data >> 8);
    spi_write_byte(data);
    LCD_CS(1);
}

/* ---------- SPI & GPIO 初始化 ---------- */
static void spi2_init(void) {
    spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOF);
    rcu_periph_clock_enable(RCU_SPI2);

    /* PF7=SCK, PF9=MOSI, 复用 AF5 */
    gpio_af_set(GPIOF, GPIO_AF_5, GPIO_PIN_7 | GPIO_PIN_9);
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7 | GPIO_PIN_9);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 | GPIO_PIN_9);

    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
    /* ?? 关键：ST7789 最常用的 Mode 0 */
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;        // SPI 时钟分频，可调整
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);
    spi_enable(SPI2);
}

static void lcd_gpio_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOF);

    /* DC=PA6, CS=PA4, BLK=PA7 */
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7);

    /* RST=PF10 */
    gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_10);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    LCD_CS(1);
    LCD_BLK(1);   /* 背光常亮 */
}

static void lcd_reset(void) {
    LCD_RST(1);
    delay_ms(10);
    LCD_RST(0);
    delay_ms(10);
    LCD_RST(1);
    delay_ms(120);              // 等待屏幕稳定
}

/* ---------- 区域、绘点、填充 ---------- */
void lcd_set_region(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_cmd(0x2A);        // 列地址
    lcd_write_data16(x1);
    lcd_write_data16(x2);
    lcd_write_cmd(0x2B);        // 行地址
    lcd_write_data16(y1);
    lcd_write_data16(y2);
    lcd_write_cmd(0x2C);        // 写内存
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color) {
    lcd_set_region(x, y, x, y);
    lcd_write_data16(color);
}

void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    uint32_t num = (x2 - x1 + 1) * (y2 - y1 + 1);
    lcd_set_region(x1, y1, x2, y2);
    LCD_DC(1);
    LCD_CS(0);
    while(num--) {
        spi_write_byte(color >> 8);
        spi_write_byte(color);
    }
    LCD_CS(1);
}

/* ---------- ST7789 初始化 ---------- */
void lcd_init(void) {
    lcd_gpio_init();            // 背光会在此点亮
    spi2_init();
    lcd_reset();

    lcd_write_cmd(0x11);        // Sleep out
    delay_ms(120);

    lcd_write_cmd(0x36);        // MADCTL: 扫描方向
    lcd_write_data(0x00);       // 默认竖屏，若不正常可改为 0x08 等尝试

    lcd_write_cmd(0x3A);        // 像素格式
    lcd_write_data(0x05);       // 16bit/pixel (RGB565)

    lcd_write_cmd(0xB2);        // Porch control
    lcd_write_data(0x0C);
    lcd_write_data(0x0C);
    lcd_write_data(0x00);
    lcd_write_data(0x33);
    lcd_write_data(0x33);

    lcd_write_cmd(0xB7);        // Gate control
    lcd_write_data(0x35);

    lcd_write_cmd(0xBB);        // VCOM
    lcd_write_data(0x19);

    lcd_write_cmd(0xC0);        // LCM control
    lcd_write_data(0x2C);

    lcd_write_cmd(0xC2);        // VDV VRH
    lcd_write_data(0x01);

    lcd_write_cmd(0xC3);        // VRH
    lcd_write_data(0x12);

    lcd_write_cmd(0xC4);        // VDV
    lcd_write_data(0x20);

    lcd_write_cmd(0xC6);        // Frame rate
    lcd_write_data(0x0F);

    lcd_write_cmd(0xD0);        // Power control 1
    lcd_write_data(0xA4);
    lcd_write_data(0xA1);

    lcd_write_cmd(0xE0);        // Positive voltage gamma
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0D);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2B);
    lcd_write_data(0x3F);
    lcd_write_data(0x54);
    lcd_write_data(0x4C);
    lcd_write_data(0x18);
    lcd_write_data(0x0D);
    lcd_write_data(0x0B);
    lcd_write_data(0x1F);
    lcd_write_data(0x23);

    lcd_write_cmd(0xE1);        // Negative voltage gamma
    lcd_write_data(0xD0);
    lcd_write_data(0x04);
    lcd_write_data(0x0C);
    lcd_write_data(0x11);
    lcd_write_data(0x13);
    lcd_write_data(0x2C);
    lcd_write_data(0x3F);
    lcd_write_data(0x44);
    lcd_write_data(0x51);
    lcd_write_data(0x2F);
    lcd_write_data(0x1F);
    lcd_write_data(0x1F);
    lcd_write_data(0x20);
    lcd_write_data(0x23);

    lcd_write_cmd(0x21);        // Display inversion on
    lcd_write_cmd(0x29);        // Display on
}