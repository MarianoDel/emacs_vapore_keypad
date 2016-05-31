/**
  ******************************************************************************
  * @file    Template_2/stm32f0_uart.c
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   UART functions.
  ******************************************************************************
  * @attention
  *
  * Use this functions to configure serial comunication interface (UART).
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "hard.h"
#include "comm.h"
#include <string.h>
#include "flash_program.h"




//--- Private typedef ---//
//--- Private define ---//
//--- Private macro ---//
#define BUFFRX_DIM 96
#define BUFFTX_DIM 64

//#define USE_USARTx_TIMEOUT

#ifdef USE_USARTx_TIMEOUT
#define BUFFRX_TIMEOUT 200 //200ms
#define BUFFTX_TIMEOUT 200 //200ms
#endif


//--- VARIABLES EXTERNAS ---//
extern volatile unsigned char *pbuffrx;
extern mem_bkp_typedef memory_backup;
extern volatile unsigned char binary_full;




//--- Private variables ---//
//Reception buffer.
unsigned char buffrx[BUFFRX_DIM];
//unsigned char buffrx_cpy[BUFFRX_DIM];
//unsigned char *pbuffrx_cpy;
volatile unsigned char buffrx_ready = 0;
volatile unsigned short binary_bytes = 1024;

#ifdef USE_USARTx_TIMEOUT
unsigned char buffrx_timeout = 0;
#endif
//Transmission buffer.
unsigned char bufftx[BUFFTX_DIM];
unsigned char * pbufftx;
unsigned char * pbufftx2;
#ifdef USE_USARTx_TIMEOUT
unsigned char bufftx_timeout = 0;
#endif
//--- Private function prototypes ---//
//--- Private functions ---//

//inline void UpdateUart(void)
unsigned char UpdateUart(unsigned char last_state)
{
	if (buffrx_ready)
	{
		buffrx_ready  = 0;
		last_state = InterpretarMsg (last_state, (char *) buffrx);
		memset(buffrx, '\0', sizeof(buffrx));
	}
	return last_state;
}

void Usart_Time_1ms (void)
{
#ifdef USE_USARTx_TIMEOUT
	if (buffrx_timeout > 1)
		buffrx_timeout--; //Se detiene con buffrx_timeout = 1.

	if (bufftx_timeout > 1)
		bufftx_timeout--; //Se detiene con bufftx_timeout = 1.
#endif
}

void UsartRxBinary(void)
{
	binary_bytes = 0;
	binary_full = 0;
}
//-------------------------------------------//
// @brief  UART configure.
// @param  None
// @retval None
//------------------------------------------//
void USARTx_IRQHandler(void)
{
	unsigned char dummy = 0;
	/* USART in mode Transmitter -------------------------------------------------*/
	if (USART_GetITStatus(USARTx, USART_IT_TXE) == SET)
	{
		if (*pbufftx2 != 0)
		{
			//GPIOA_PIN0_ON;

#ifdef USE_USARTx_TIMEOUT
			bufftx_timeout = BUFFRX_TIMEOUT;
#endif

			USARTx->TDR = *pbufftx2 & 0x1FF;

			if (pbufftx2 < &bufftx[BUFFTX_DIM])
			{
				pbufftx2++;

				if (pbuffrx == &buffrx[BUFFRX_DIM])
					pbuffrx = buffrx;
			}
			else
				pbufftx2 = bufftx;

		}
		else
		{
			//GPIOA_PIN0_OFF;

#ifdef USE_USARTx_TIMEOUT
			bufftx_timeout = 0;
#endif
			USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);

			pbufftx2 = bufftx;
			pbufftx = bufftx;
		}


	}

	/* USART in mode Receiver --------------------------------------------------*/
	if (USART_GetITStatus(USARTx, USART_IT_RXNE) == SET)
	{

#ifdef USE_USARTx_TIMEOUT
		buffrx_timeout = BUFFRX_TIMEOUT;
#endif
		//formato binario
		if (binary_bytes < 1024)
		{
			memory_backup.v_bkp_8u[binary_bytes] = USARTx->RDR & 0x0FF;
			binary_bytes++;
			if (binary_bytes == 1024)
				binary_full = 1;
		}
		else
		{
			//Saving incoming data.
			*pbuffrx = USARTx->RDR & 0x1FF;

			if (pbuffrx < &buffrx[BUFFRX_DIM])
			{
				if (*pbuffrx == '\n')
				{

					*pbuffrx = 0;
					buffrx_ready = 1;
					pbuffrx = buffrx;
				}
				else
				{
					pbuffrx++;

					if (pbuffrx == &buffrx[BUFFRX_DIM])
						pbuffrx = buffrx;
				}
			}
			else
				pbuffrx = buffrx;
		}
		USARTx->RQR |= 0x08;
	}

	if ((USART_GetITStatus(USARTx, USART_IT_ORE) == SET) ||
			(USART_GetITStatus(USARTx, USART_IT_NE) == SET) ||
			(USART_GetITStatus(USARTx, USART_IT_FE) == SET))
	{
		USARTx->ICR |= 0x0e;
		dummy = USARTx->RDR;
		return;
	}
}

void USART_Config(void)
{
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_AHBPeriphClockCmd(USARTx_TX_GPIO_CLK | USARTx_RX_GPIO_CLK, ENABLE);

  /* Enable USART clock */
  USARTx_APBPERIPHCLOCK(USARTx_CLK, ENABLE);

  /* Connect PXx to USARTx_Tx */
  GPIO_PinAFConfig(USARTx_TX_GPIO_PORT, USARTx_TX_SOURCE, USARTx_TX_AF);

  /* Connect PXx to USARTx_Rx */
  GPIO_PinAFConfig(USARTx_RX_GPIO_PORT, USARTx_RX_SOURCE, USARTx_RX_AF);

  /* Configure USART Tx and Rx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
  GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
  GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStructure);

  /* USARTx configuration ----------------------------------------------------*/
  /* USARTx configured as follow:
  - BaudRate = 9600 baud
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);

  /* NVIC configuration */
  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  //--- Initialize pointers ---//
  pbuffrx = buffrx;
  pbufftx = bufftx;
  pbufftx2 = bufftx;
  buffrx_ready = 0;

#ifdef USE_USARTx_TIMEOUT
  buffrx_timeout = 0;
  bufftx_timeout = 0;
#endif

  /* Enable USART */
  USART_Cmd(USARTx, ENABLE);

  //--- Enable receiver interrupt ---//
  USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
}

/*
void USARTx_receive (void)
{

	if (buffrx_ready)
	{

		//USARTx_Send((char *)&buffrx_cpy[0]);

		if (strncmp((char *)&buffrx_cpy[0], "keepalive", sizeof("keepalive") - 1) == 0)
		{
			Wait_ms(30);
			USARTx_Send("ant4,003.44,023.00,004.50,065.00\r\n");
			Wait_ms(30);
		}



		buffrx_ready = 0;
	}

#ifdef USE_USARTx_TIMEOUT
	if (buffrx_timeout == 1)
	{
		buffrx_timeout = 0;

		pbuffrx = buffrx;
	}

	if (bufftx_timeout == 1)
	{
		bufftx_timeout = 0;

		pbufftx = bufftx;
	}
#endif
}
*/
unsigned char USARTx_Send(char * send)
{
	unsigned char i;

	i = strlen(send);

	if ((pbufftx + i) < &bufftx[BUFFTX_DIM])
	{

		strcpy((char*)pbufftx, (const char *)send);
		pbufftx += i;

		USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);

	}
	else
		//return END_ERROR;
		return 0;


	//return WORKING;
	return 1;
}

void USARTx_Send_UINT(unsigned char * tosend, unsigned short lenght)
{
	unsigned short i;
	//desactivo la interrupcion
	USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);

	//if (USART_GetITStatus(USARTx, USART_IT_TXE) == SET)

	for (i = 0; i < lenght; i++)
	{
		while ((USARTx->ISR & 0x0080) == 0);
		USARTx->TDR = *tosend & 0x0FF;
		tosend++;
	}
}

//--- end of file ---//

