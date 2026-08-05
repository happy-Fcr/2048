#ifndef __ADC_H
#define __ADC_H

#include "gd32f4xx.h"

void ADC_Init(void);
uint16_t ADC_Get_X(void);
uint16_t ADC_Get_Y(void);
uint8_t Joystick_Get_Direction(void);  // 返回：1上,2下,3左,4右,0无

#endif