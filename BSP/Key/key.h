#ifndef __KEY_H
#define	__KEY_H

#include "stm32f10x.h"

//KEY端口定义
#define KEY_RCC     RCC_APB2Periph_GPIOA                           
#define KEY_PORT	GPIOA  
#define KEY0        GPIO_Pin_4    
#define KEY1        GPIO_Pin_5
#define KEY2        GPIO_Pin_6    
#define KEY3        GPIO_Pin_7 

//按键标置
#define KEY_DOWN	0
#define KEY_UP		1

void Key_GPIO_Config(void);
u8 Key_Down_Detection(GPIO_TypeDef* GPIOx, u16 GPIO_Pin);
u16 Key_Down_Scan(void);

#endif 

