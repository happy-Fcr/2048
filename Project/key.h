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

#define KEY_NONE  0U
#define KEY_UP    1U
#define KEY_DOWN  2U
#define KEY_LEFT  3U
#define KEY_RIGHT 4U

#endif /* __KEY_H */
