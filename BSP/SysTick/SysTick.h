#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init_Config(void);
void delay_ms(u32 nms);
void Delay_nS(u32 ns);
void delay_us(__IO uint32_t n);
void _delay_Nms(u32 nTime);

#endif 

