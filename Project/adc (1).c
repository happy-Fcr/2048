#include "adc.h"
#include "systick.h"
#include "key.h"

static uint16_t adc_read_channel(uint8_t channel)
{
    adc_regular_channel_config(ADC0, 0, channel, ADC_SAMPLETIME_15);
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    while (!adc_flag_get(ADC0, ADC_FLAG_EOC));
    return adc_regular_data_read(ADC0);
}

void ADC_Init(void)
{
    // 使能时钟
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);

    // 配置引脚模拟输入
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);
    gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);

    // ADC 复位
    adc_deinit();

    // 注意：跳过显式时钟配置，使用默认值（通常来自PCLK2）
    // adc_clock_config(ADC0, ADC_CLK_SYNC_PCLK_DIV4);  // 若未定义可注释

    // 配置扫描模式、数据对齐
    adc_special_function_config(ADC0, ADC_SCAN_MODE, DISABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);

    // 规则组通道长度
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);

    // 禁用外部触发，使用软件触发
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, DISABLE);

    // 使能 ADC
    adc_enable(ADC0);
    delay_1ms(10);

    // 校准
    adc_calibration_enable(ADC0);
}

uint16_t ADC_Get_X(void)
{
    return adc_read_channel(ADC_CHANNEL_11);  // PC1
}

uint16_t ADC_Get_Y(void)
{
    return adc_read_channel(ADC_CHANNEL_1);   // PA1
}

uint8_t Joystick_Get_Direction(void)
{
    uint16_t x = ADC_Get_X();
    uint16_t y = ADC_Get_Y();
    uint16_t deadzone = 500;
    uint16_t center = 2048;

    if (x < center - deadzone) return KEY_LEFT;
    if (x > center + deadzone) return KEY_RIGHT;
    if (y < center - deadzone) return KEY_UP;
    if (y > center + deadzone) return KEY_DOWN;
    return KEY_NONE;
}