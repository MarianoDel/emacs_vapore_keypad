/**
  ******************************************************************************
  * @file    Template_2/stm32f0x_tim.c
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   TIM functions.
  ******************************************************************************
  * @attention
  *
  * Use this functions to configure timers.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0x_tim.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_misc.h"
#include "usart.h"
#include "hard.h"
#include "rws317.h"

//desde que saque main.h
#include "stm32f0xx_gpio.h"
//fin desde que saque main.h


//--- VARIABLES EXTERNAS ---//
extern volatile unsigned char timer_1seg;
extern volatile unsigned short timer_led_comm;
extern volatile unsigned short timer_standby;
extern volatile unsigned short wait_ms_var;
extern volatile short v_samples1[];
extern volatile short v_samples2[];
extern volatile unsigned char update_samples;
extern volatile unsigned char buff_in_use;
extern volatile unsigned short v_pwm[];

//--- VARIABLES GLOBALES ---//

volatile unsigned short timer_1000 = 0;
volatile unsigned char new_sample = 0;

const unsigned short sin16 [] = {  256, 354, 436, 492, 511, 492, 436, 354,
								   256, 158, 76, 20, 1, 20, 76, 158};

//const short sin16_short [] = {  0, 98, 180, 236, 255, 236, 180, 98,
//								   0, -98, -180, -236, -255, -236, -180, -98};

const short sin16_short [] = {  0, 12539, 23170, 30273, 32767, 30273, 23170, 12539,
								   0, -12539, -23170, -30273, -32767, -30273, -23170, -12539};

const short sin40_short [] = {  0, 5126, 10126, 14876, 19260, 23170, 26509, 29196,
								31163, 32364, 32767, 32364, 31163, 29196, 26509, 23170,
								19260, 14876, 10126, 5126, 0, -5126, -10126, -14876,
								-19260, -23170, -26509, -29196, -31163, -32364, -32767, -32364,
								-31163, -29196, -26509, -23170, -19260, -14876, -10126, -5126 };

unsigned char sample_index = 0;
#define SAMPLE_MAX_INDEX	40


//--- FUNCIONES DEL MODULO ---//
void LoadSine(short * v)
{
	unsigned char i;

	for (i = 0; i < 16; i++)
	{
		if (sample_index < SAMPLE_MAX_INDEX)
		{
			*(v+i) = sin40_short[sample_index];
			sample_index++;
		}
		else
		{
			*(v+i) = sin40_short[0];
			sample_index = 1;
		}
	}
}

inline void Power_Ampli_Ena (void)
{
	//hab general de OC
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void Ampli_to_Audio (void)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;

	TIM1->ARR = 511;
	TIM1->CNT = 0;
	TIM1->PSC = 5;	//prescaler divido 48MHz / (1 + 5)
	// Enable timer ver UDIS
	TIM1->DIER |= TIM_DIER_UIE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void Ampli_to_Sirena (void)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;

	TIM1->ARR = TIM1_ARR;
	TIM1->CNT = 0;
	TIM1->PSC = 47;	//prescaler divido 48MHz / (1 + 47)

	TIM1->DIER &= ~TIM_DIER_UIE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

inline void Power_Ampli_Disa (void)
{
	//hab general de OC
	TIM1->BDTR &= 0x7FFF;
	TIM1->CR1 &= ~TIM_CR1_CEN;
}

void ChangeAmpli(unsigned short freq, unsigned short pwm)
{
	TIM1->ARR = freq;
	TIM1->CCR1 = pwm;
}

void Update_TIM1_CH1 (unsigned short a)
{
	TIM1->CCR1 = a;
}

void Update_TIM3_CH1 (unsigned short a)
{
	TIM3->CCR1 = a;
}

void Update_TIM3_CH2 (unsigned short a)
{
	TIM3->CCR2 = a;
}

void Wait_ms (unsigned short wait)
{
	wait_ms_var = wait;

	while (wait_ms_var);
}

//-------------------------------------------//
// @brief  TIM configure.
// @param  None
// @retval None
//------------------------------------------//
void TIM3_IRQHandler (void)	//1 ms
{

	// Usart_Time_1ms ();

	if (timer_1seg)
	{
		if (timer_1000)
			timer_1000--;
		else
		{
			timer_1seg--;
			timer_1000 = 1000;
		}
	}

	if (timer_led_comm)
		timer_led_comm--;

	if (timer_standby)
		timer_standby--;

	//bajar flag
	if (TIM3->SR & 0x01)	//bajo el flag
		TIM3->SR = 0x00;
}

void TIM1_BRK_UP_TRG_COM_IRQHandler (void)	//16KHz
{
	//cuando arranca siempre in_buff == 1;
	if (buff_in_use == 1)
	{
		if (new_sample < 16)
		{
			//v_pwm[new_sample] = (v_samples1[new_sample] >> 7) + 256;
			//TIM1->CCR1 = v_pwm[new_sample];
			TIM1->CCR1 = (v_samples1[new_sample] >> 7) + 256;
			new_sample++;
		}
		else
		{
			//tengo que cambiar de buffer
			buff_in_use = 2;
			update_samples++;
			TIM1->CCR1 = (v_samples2[0] >> 7) + 256;
			new_sample = 1;
		}
	}
	else if (buff_in_use == 2)
	{
		if (new_sample < 16)
		{
			//v_pwm[new_sample] = (v_samples2[new_sample] >> 7) + 256;
			//TIM1->CCR1 = v_pwm[new_sample];
			TIM1->CCR1 = (v_samples2[new_sample] >> 7) + 256;
			new_sample++;
		}
		else
		{
			//tengo que cambiar de buffer
			buff_in_use = 1;
			update_samples++;
			//v_pwm[0] = (v_samples1[0] >> 7) + 256;
			//TIM1->CCR1 = v_pwm[0];
			TIM1->CCR1 = (v_samples1[0] >> 7) + 256;
			new_sample = 1;
		}
	}


	/*
	if (new_sample < 16)
	{
		TIM1->CCR1 = sin16[new_sample];
		new_sample++;
	}
	else
	{
		TIM1->CCR1 = sin16[0];
		new_sample = 1;
	}
	*/
	//bajar flag
	if (TIM1->SR & 0x01)	//bajo el flag
		TIM1->SR = 0x00;

}

void TIM_1_Init (void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if (!RCC_TIM1_CLK)
		RCC_TIM1_CLK_ON;

	/* Connect PXx to TIM1_CH1N */
	GPIO_PinAFConfig(TIM1_CH1N_GPIO_PORT, TIM1_CH1N_SOURCE, TIM1_CH1N_AF);

	/* Connect PXx to TIM1_CH1 */
	GPIO_PinAFConfig(TIM1_CH1_GPIO_PORT, TIM1_CH1_SOURCE, TIM1_CH1_AF);

	/* Configure TIM1 CH1N and CH1 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = TIM1_CH1N_PIN;
	GPIO_Init(TIM1_CH1N_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = TIM1_CH1_PIN;
	GPIO_Init(TIM1_CH1_GPIO_PORT, &GPIO_InitStructure);

	//Configuracion del timer.
	//TIM1->CR1 = 0x00;		//clk int / 1; upcounting ARPE bit in the TIMx_CR1 para pwm mode 1
	TIM1->CR1 = TIM_CR1_ARPE;
	TIM1->CR2 = 0x00;		//igual al reset

	//OJO si es PWM OCxPE debe ser 1
	TIM1->CCMR1 = 0x0068;			//CH1 output PWM mode 1 update on UE
	//TIM1->CCMR1 = 0x0028;			//CH1 output; inactive on output match; update on UE	NO FUNCIONA

	//CH1 y CH1N negado enable on pin CC1P CC1NP active high
	TIM1->CCER |= TIM_CCER_CC1E |  TIM_CCER_CC1NE;
	//TIM1->CCER |= TIM_CCER_CC1E |  TIM_CCER_CC1NE | TIM_CCER_CC1P;	//salen las 2 en fase
	//CH1 y CH1N negado enable on pin CC1P CC1NP active low
	//TIM1->CCER |= TIM_CCER_CC1E |  TIM_CCER_CC1NE | TIM_CCER_CC1P | TIM_CCER_CC1NP;	//contrafase activo al reves

	//hab general de OC y estado inhabilitado alto
	//TIM1->BDTR |= TIM_BDTR_MOE;
	//hab general de OC y estado inhabilitado bajo
	TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSI;

	TIM1->ARR = TIM1_ARR;
	TIM1->CNT = 0;
	TIM1->PSC = 47;	//prescaler divido 48MHz / (1 + 47)
	//TIM3->PSC = 0;	//prescaler divido 48MHz / (1 + 0)
	//TIM3->EGR = TIM_EGR_UG;

	// Enable timer ver UDIS
	//TIM3->DIER |= TIM_DIER_UIE;
	TIM1->CR1 |= TIM_CR1_CEN;

	//Timer sin Int
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


void TIM_3_Init (void)
{

	//NVIC_InitTypeDef NVIC_InitStructure;

	if (!RCC_TIM3_CLK)
		RCC_TIM3_CLK_ON;

	//Configuracion del timer.
	TIM3->CR1 = 0x00;		//clk int / 1; upcounting
	TIM3->CR2 = 0x00;		//igual al reset
	TIM3->CCMR1 = 0x7070;			//CH2 y CH1 output PWM mode 2
	//TIM3->CCER |= TIM_CCER_CC2E | TIM_CCER_CC1E;	//CH2 y CH1 enable on pin
	TIM3->CCER |= TIM_CCER_CC2E | TIM_CCER_CC1E |  TIM_CCER_CC1P;	//CH2 y CH1 negado enable on pin
	TIM3->ARR = TIM3_ARR;
	TIM3->CNT = 0;
	TIM3->PSC = 47;	//prescaler divido 48MHz / (1 + 47)
	//TIM3->PSC = 0;	//prescaler divido 48MHz / (1 + 0)
	//TIM3->EGR = TIM_EGR_UG;

	// Enable timer ver UDIS
	//TIM3->DIER |= TIM_DIER_UIE;
	TIM3->CR1 |= TIM_CR1_CEN;

	//Timer sin Int
	//NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);
}

inline void TIM14_IC_CH1_OFF (void)
{
	//desa int
	//TIM14->DIER |= TIM_DIER_UIE;
	TIM14->DIER &= 0xFFFD;
}

inline void TIM14_IC_CH1_ON (void)
{
	//ena int
	TIM14->DIER |= TIM_DIER_CC1IE;
}

void TIM14_IRQHandler (void)	//100uS
{
	//reload or overcaptured
	if (((TIM14->SR & 0x01) != 0) || ((TIM14->SR & 0x20) != 0))
		TIM14->SR = 0x00;	//bajar flag

	//IC OJO puede ser error
	if ((TIM14->SR & 0x02) != 0)
	{
		//ver pag 356
		Timer_Interrupt_Handler (TIM14->CCR1);
		TIM14->SR = 0x00;
	}
}


void TIM_14_Init (void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if (!RCC_TIM14_CLK)
		RCC_TIM14_CLK_ON;

	/* Connect PXx to TIM14_CH1 */
	GPIO_PinAFConfig(TIM14_CH1_GPIO_PORT, TIM14_CH1_SOURCE, TIM14_CH1_AF);

	/* Configure TIM1 CH1N and CH1 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = TIM14_CH1_PIN;
	GPIO_Init(TIM14_CH1_GPIO_PORT, &GPIO_InitStructure);

	//Configuracion del timer.
	//TIM14->ARR = 2000; //10m
	TIM14->CR1 = 0;
	TIM14->ARR = 0xFFFF;		//modo continuo
	TIM14->CNT = 0;
	TIM14->PSC = 47;	//1us por tick

	//N = 8 filter; sin prescaler; input capture
	//TIM14->CCMR1 = 0x0031;	//filtro OK
	//TIM14->CCMR1 = 0x0021;	//filtro OK
	//TIM14->CCMR1 = 0x0041;		//filtro OK
	TIM14->CCMR1 = 0x0071;		//filtro OK
	//TIM14->CCMR1 = 0x0001;		//sin filtro
	TIM14->CCER |= TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NP;	//input capture CH1 any edge

	//TIM14->EGR = TIM_EGR_UG;

	//Interrupts
	// Enable timer ver UDIS
	//TIM14->DIER |= TIM_DIER_UIE;
	TIM14->DIER |= TIM_DIER_CC1IE;
	TIM14->CR1 |= TIM_CR1_CEN;

	NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM16_IRQHandler (void)	//100uS
{

	if (TIM16->SR & 0x01)
		//bajar flag
		TIM16->SR = 0x00;
}


void TIM_16_Init (void)
{

	NVIC_InitTypeDef NVIC_InitStructure;

	if (!RCC_TIM16_CLK)
		RCC_TIM16_CLK_ON;

	//Configuracion del timer.
	TIM16->ARR = 2000; //10m
	TIM16->CNT = 0;
	TIM16->PSC = 479;
	TIM16->EGR = TIM_EGR_UG;

	// Enable timer ver UDIS
	TIM16->DIER |= TIM_DIER_UIE;
	TIM16->CR1 |= TIM_CR1_CEN;

	NVIC_InitStructure.NVIC_IRQChannel = TIM16_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void TIM17_IRQHandler (void)	//100uS
{

	//if (GPIOA_PIN0_OUT)
	//	GPIOA_PIN0_OFF;
	//else
	//	GPIOA_PIN0_ON;

	if (TIM17->SR & 0x01)
		//bajar flag
		TIM17->SR = 0x00;
}


void TIM_17_Init (void)
{

	NVIC_InitTypeDef NVIC_InitStructure;

	if (!RCC_TIM17_CLK)
		RCC_TIM17_CLK_ON;

	//Configuracion del timer.
	TIM17->ARR = 2000; //10m
	TIM17->CNT = 0;
	TIM17->PSC = 479;
	TIM17->EGR = TIM_EGR_UG;

	// Enable timer ver UDIS
	TIM17->DIER |= TIM_DIER_UIE;
	TIM17->CR1 |= TIM_CR1_CEN;

	NVIC_InitStructure.NVIC_IRQChannel = TIM17_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//--- end of file ---//


