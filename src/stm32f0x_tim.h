/**
  ******************************************************************************
  * @file    Template_2/main.h
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   Header for tim module
  ******************************************************************************
  * @attention
  *
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F0X_TIM_H
#define __STM32F0X_TIM_H

//--- Exported types ---//
//--- Exported constants ---//
//--- Exported macro ---//
#define RCC_TIM1_CLK 		(RCC->APB2ENR & 0x00000800)
#define RCC_TIM1_CLK_ON 	RCC->APB2ENR |= 0x00000800
#define RCC_TIM1_CLK_OFF 	RCC->APB2ENR &= ~0x00000800

#define RCC_TIM3_CLK 		(RCC->APB1ENR & 0x00000002)
#define RCC_TIM3_CLK_ON 	RCC->APB1ENR |= 0x00000002
#define RCC_TIM3_CLK_OFF 	RCC->APB1ENR &= ~0x00000002

#define RCC_TIM14_CLK 		(RCC->APB1ENR & 0x00000100)
#define RCC_TIM14_CLK_ON 	RCC->APB1ENR |= 0x00000100
#define RCC_TIM14_CLK_OFF 	RCC->APB1ENR &= ~0x00000100

#define RCC_TIM15_CLK 		(RCC->APB2ENR & 0x00010000)
#define RCC_TIM15_CLK_ON 	RCC->APB2ENR |= 0x00010000
#define RCC_TIM15_CLK_OFF 	RCC->APB2ENR &= ~0x00010000

#define RCC_TIM16_CLK 		(RCC->APB2ENR & 0x00020000)
#define RCC_TIM16_CLK_ON 	RCC->APB2ENR |= 0x00020000
#define RCC_TIM16_CLK_OFF 	RCC->APB2ENR &= ~0x00020000

#define RCC_TIM17_CLK 		(RCC->APB2ENR & 0x00040000)
#define RCC_TIM17_CLK_ON 	RCC->APB2ENR |= 0x00040000
#define RCC_TIM17_CLK_OFF 	RCC->APB2ENR &= ~0x00040000

#define TIM3_ARR 666
#define TIM1_ARR 666

//en PA7 y PA8
#define TIM1_CH1N_PIN			GPIO_Pin_7
#define TIM1_CH1N_GPIO_PORT		GPIOA
#define TIM1_CH1N_GPIO_CLK		RCC_AHBPeriph_GPIOA
#define TIM1_CH1N_SOURCE		GPIO_PinSource7
#define TIM1_CH1N_AF			GPIO_AF_2

#define TIM1_CH1_PIN			GPIO_Pin_8
#define TIM1_CH1_GPIO_PORT		GPIOA
#define TIM1_CH1_GPIO_CLK		RCC_AHBPeriph_GPIOA
#define TIM1_CH1_SOURCE			GPIO_PinSource8
#define TIM1_CH1_AF				GPIO_AF_2

//en PA4
#define TIM14_CH1_PIN			GPIO_Pin_4
#define TIM14_CH1_GPIO_PORT		GPIOA
#define TIM14_CH1_GPIO_CLK		RCC_AHBPeriph_GPIOA
#define TIM14_CH1_SOURCE		GPIO_PinSource4
#define TIM14_CH1_AF			GPIO_AF_4


#define Timer_1_Init() TIM_1_Init()
#define Timer_1b_Init()	TIM_3_Init()
#define Timer_2_Init() TIM_14_Init()
#define Timer_3_Init() TIM_16_Init()
#define Timer_4_Init() TIM_17_Init()

#define FREQ_1000HZ	1000
#define FREQ_800HZ	1250

#define DUTY_50_1000	500
#define DUTY_50_800		625

//--- Exported functions ---//
void TIM14_IC_CH1_OFF (void);
void TIM14_IC_CH1_ON (void);
void Power_Ampli_Ena (void);
void Power_Ampli_Disa (void);
void ChangeAmpli(unsigned short, unsigned short);
void TIM_1_Init (void);
void TIM3_IRQHandler (void);
void TIM_3_Init(void);
void TIM14_IRQHandler (void);
void TIM_14_Init(void);
void TIM16_IRQHandler (void);
void TIM_16_Init(void);
void TIM17_IRQHandler (void);
void TIM_17_Init(void);
void Update_TIM3_CH1 (unsigned short);
void Update_TIM3_CH2 (unsigned short);
void Update_TIM1_CH1 (unsigned short);

void Wait_ms (unsigned short wait);

void Ampli_to_Audio (void);
void Ampli_to_Sirena (void);
void LoadSine(short *);

#define CodeInterruptDisa() TIM14_IC_CH1_OFF()
#define CodeInterruptEna() TIM14_IC_CH1_ON()

#endif
//--- End ---//


//--- END OF FILE ---//
