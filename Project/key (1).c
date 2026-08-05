/*========================= key.c =========================*/

#include "game.h"

void delay_1ms(uint32_t ms);

void key_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOG);

    gpio_mode_set(GPIOA, GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_0);

    gpio_mode_set(GPIOB, GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_15 | GPIO_PIN_2);

    gpio_mode_set(GPIOG, GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_9);
}

int key_scan(void)
{
    if(gpio_input_bit_get(GPIOG, GPIO_PIN_9)==0)
    {
        delay_1ms(10);

        if(gpio_input_bit_get(GPIOG, GPIO_PIN_9)==0)
        {
            while(gpio_input_bit_get(GPIOG, GPIO_PIN_9)==0);
            return KEY_UP;
        }
    }

    if(gpio_input_bit_get(GPIOB, GPIO_PIN_15)==0)
    {
        delay_1ms(10);

        if(gpio_input_bit_get(GPIOB, GPIO_PIN_15)==0)
        {
            while(gpio_input_bit_get(GPIOB, GPIO_PIN_15)==0);
            return KEY_DOWN;
        }
    }

    if(gpio_input_bit_get(GPIOA, GPIO_PIN_0)==0)
    {
        delay_1ms(10);

        if(gpio_input_bit_get(GPIOA, GPIO_PIN_0)==0)
        {
            while(gpio_input_bit_get(GPIOA, GPIO_PIN_0)==0);
            return KEY_LEFT;
        }
    }

    if(gpio_input_bit_get(GPIOB, GPIO_PIN_2)==0)
    {
        delay_1ms(10);

        if(gpio_input_bit_get(GPIOB, GPIO_PIN_2)==0)
        {
            while(gpio_input_bit_get(GPIOB, GPIO_PIN_2)==0);
            return KEY_RIGHT;
        }
    }

    return 0;
}