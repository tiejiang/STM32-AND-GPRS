#ifndef __LED_H__
#define __LED_H__
	 
#include "stm32f10x.h"

//LED¶Ë¿Ú¶¨Òå
#define LED_RCC     RCC_APB2Periph_GPIOB                            
#define LED_PORT	GPIOB    
#define LED0        GPIO_Pin_4    
#define LED1        GPIO_Pin_5    
#define LED2        GPIO_Pin_6    
//#define LED3        GPIO_Pin_7
#define LED4        GPIO_Pin_12    
#define LED5        GPIO_Pin_13    
#define LED6        GPIO_Pin_14    
#define LED7        GPIO_Pin_15

//#define LED_ALL     LED0 | LED1 | LED2 | LED3 | LED4 | LED5 | LED6 | LED7 
#define LED_ALL     LED0 | LED1 | LED2 | LED4 | LED5 | LED6 | LED7
#define PWR_MG323   GPIO_Pin_9

void GPIO_Config(void);
void LED0_ON(void);
void LED0_OFF(void);
void LED1_ON(void);
void LED1_OFF(void);
void LED2_ON(void);
void LED2_OFF(void);
//void LED3_ON(void);
//void LED3_OFF(void);
void LED4_ON(void);
void LED4_OFF(void);
void LED5_ON(void);
void LED5_OFF(void);
void LED6_ON(void);
void LED6_OFF(void);
void LED7_ON(void);
void LED7_OFF(void);
void LED_ALL_ON(void);
void LED_ALL_OFF(void);
		 				    
#endif
