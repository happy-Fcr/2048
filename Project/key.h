#ifndef __KEY_H
#define __KEY_H

#include "gd32f4xx.h"

#define KEY_UP_PORT     GPIOG
#define KEY_UP_PIN      GPIO_PIN_9
#define KEY_DOWN_PORT   GPIOB
#define KEY_DOWN_PIN    GPIO_PIN_15
#define KEY_LEFT_PORT   GPIOA
#define KEY_LEFT_PIN    GPIO_PIN_0
#define KEY_RIGHT_PORT  GPIOB
#define KEY_RIGHT_PIN   GPIO_PIN_2

#define KEY_NONE    0
#define KEY_UP      1
#define KEY_DOWN    2
#define KEY_LEFT    3
#define KEY_RIGHT   4

void KEY_Init(void);
uint8_t KEY_GetPressed(void);

#endif