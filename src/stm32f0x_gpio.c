/**
  ******************************************************************************
  * @file    Template_2/stm32f0_gpio.c
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   GPIO functions.
  ******************************************************************************
  * @attention
  *
  * Use this functions to configure global inputs/outputs.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0x_gpio.h"
#include "stm32f0xx_gpio.h"



//--- Private typedef ---//
//--- Private define ---//
//--- Private macro ---//
//--- Private variables ---//
//--- Private function prototypes ---//
//--- Private functions ---//

//-------------------------------------------//
// @brief  GPIO configure.
// @param  None
// @retval None
//------------------------------------------//
void GPIO_Config (void)
{
	unsigned long temp;

	//--- MODER ---//
	//00: Input mode (reset state)
	//01: General purpose output mode
	//10: Alternate function mode
	//11: Analog mode

	//--- OTYPER ---//
	//These bits are written by software to configure the I/O output type.
	//0: Output push-pull (reset state)
	//1: Output open-drain

	//--- ORSPEEDR ---//
	//These bits are written by software to configure the I/O output speed.
	//x0: Low speed.
	//01: Medium speed.
	//11: High speed.
	//Note: Refer to the device datasheet for the frequency.

	//--- PUPDR ---//
	//These bits are written by software to configure the I/O pull-up or pull-down
	//00: No pull-up, pull-down
	//01: Pull-up
	//10: Pull-down
	//11: Reserved


#ifdef GPIOA_ENABLE

	//--- GPIO A ---//
	if (!GPIOA_CLK)
		GPIOA_CLK_ON;

	temp = GPIOA->MODER;	//2 bits por pin
	temp &= 0x3C3FC000;		//salidas PA0 a PA6 PA11 PA12 y PA15 entrada
	temp |= 0x01401555;
	GPIOA->MODER = temp;

	temp = GPIOA->OTYPER;	//1 bit por pin
	temp &= 0xFFFFE7C0;
	temp |= 0x00000000;		//PA0 a PA6push pull
	GPIOA->OTYPER = temp;

	temp = GPIOA->OSPEEDR;	//2 bits por pin
	temp &= 0xFC3FC000;
	temp |= 0x00000000;		//low speed
	GPIOA->OSPEEDR = temp;

	temp = GPIOA->PUPDR;	//2 bits por pin
	temp &= 0x3FFFFFFF;
	temp |= 0x40000000;
	GPIOA->PUPDR = temp;

	//Alternate Fuction
	//GPIOA->AFR[0] = 0x11000000;	//PA7 -> AF1; PA6 -> AF1


#endif

#ifdef GPIOB_ENABLE

	//--- GPIO B ---//
	if (!GPIOB_CLK)
		GPIOB_CLK_ON;

	temp = GPIOB->MODER;	//2 bits por pin
	temp &= 0xFFFFC030;	//PB0 PB1 output; PB3 PB4 PB5 alternative; PB6 output
	temp |= 0x00001A85;
	GPIOB->MODER = temp;

	temp = GPIOB->OTYPER;	//1 bit por pin
	temp &= 0xFFFFFFBC;
	temp |= 0x00000000;		//PB1 push pull
	GPIOB->OTYPER = temp;

	temp = GPIOB->OSPEEDR;	//2 bits por pin
	temp &= 0xFFFFCFF0;
	temp |= 0x00000000;		//low speed
	GPIOB->OSPEEDR = temp;

	temp = GPIOB->PUPDR;	//2 bits por pin
	temp &= 0xFFFFCFF0;
	temp |= 0x00000000;
	GPIOB->PUPDR = temp;

	//Alternate Fuction
	//GPIOB->AFR[0] = 0x11000000;	//PA7 -> AF1; PA6 -> AF1

#endif

#ifdef GPIOF_ENABLE

	//--- GPIO F ---//
	if (!GPIOF_CLK)
		GPIOF_CLK_ON;

	temp = GPIOF->MODER;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->MODER = temp;

	temp = GPIOF->OTYPER;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->OTYPER = temp;

	temp = GPIOF->OSPEEDR;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->OSPEEDR = temp;

	temp = GPIOF->PUPDR;
	temp &= 0xFFFFFFFF;
	temp |= 0x00000000;
	GPIOF->PUPDR = temp;

#endif

}
//--- end of file ---//
