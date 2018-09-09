#include "spi.h"
#include "stm32f0xx.h"

//desde que saque main.h
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_spi.h"
#include "stm32f0xx_rcc.h"

#define SPIx                             SPI1
//fin desde que saque main.h


//-------------------------- EXTERNAL VARIABLES ------------------------
extern volatile unsigned char TxBuffer_SPI [];
extern volatile unsigned char RxBuffer_SPI [];
extern volatile unsigned char *pspi_tx;
extern volatile unsigned char *pspi_rx;
extern volatile unsigned char spi_bytes_left;
//extern SPI_InitTypeDef  SPI_InitStructure;

//-------------------------- GLOBAL VARIABLES --------------------------
//SPI_InitTypeDef  SPI_InitStructure;

//--------------------------- FUNCTIONS --------------------------------

/**
  * @brief  Configures the SPI Peripheral.
  * @param  None
  * @retval None
  */
void SPI_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  //NVIC_InitTypeDef NVIC_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable the SPI periph */
  RCC_APB2PeriphClockCmd(SPIx_CLK, ENABLE);

  /* Enable SCK, MOSI, MISO and NSS GPIO clocks */
  RCC_AHBPeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);

  //traba al programa (salta hrd_exception?)
  GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
  GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);
  GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

  //ademas lo pongo en 1 PB3
  //GPIOB->BSRR = 0x00000008;

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
  GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_MISO_PIN;
  GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/

  //SPI_I2S_DeInit(SPIx);

  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DATASIZE;
  //para 4021
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  //para 74hc595 ok pero no el punto
  //SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  //SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  //SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
  //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  //SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;	//a esta velocidad no responden los 4021
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
//  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

  /* Configure the SPI interrupt priority */
  //NVIC_InitStructure.NVIC_IRQChannel = SPIx_IRQn;
  //NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
  //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //NVIC_Init(&NVIC_InitStructure);



  /* Initializes the SPI communication */
  SPI_Init(SPIx, &SPI_InitStructure);
  /* Enable the SPI peripheral */
  SPI_Cmd(SPIx, ENABLE);




  //Configuracion SPI
  SPIx->CR1 &= ~SPI_CR1_SPE;		//deshabilito periferico
  //clk / 256; master; CPOL High; CPHA second clock
  //SPIx->CR1 = 0;	//TODO: CR1 es el que tiene un error respecto de la config de arriba (verificar que puede ser) AHORRO MUCHA MEMORIA
  SPIx->CR2 = 0;
  //SPIx->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2 | SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM;
  SPIx->CR1 |= SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM;
  //thresh 8 bits; data 8 bits;
  SPIx->CR2 |= SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;

//  SPIx->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);
  SPIx->CR1 |= SPI_CR1_SPE;		//habilito periferico

}

void Send_SPI (unsigned char * p, unsigned char bytes)
{
	/* Waiting until TX FIFO is empty */
    while (SPI_GetTransmissionFIFOStatus(SPIx) != SPI_TransmissionFIFOStatus_Empty);

	pspi_tx = p;
	spi_bytes_left = bytes;

    SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, ENABLE);
}

unsigned char Send_Receive_SPI (unsigned char a)
{
	unsigned char rx;
	unsigned char j;

	//primero limpio buffer rx spi
	while ((SPIx->SR & SPI_SR_RXNE) == 1)
	{
		rx = SPI_ReceiveData8(SPIx);
	}

    while (SPI_GetTransmissionFIFOStatus(SPIx) != SPI_TransmissionFIFOStatus_Empty);
    SPI_SendData8(SPIx, a);
    //espero tener el dato en RX
    for (j = 0; j < 150; j++)
    {
    	asm("nop");
    }

    return (SPI_ReceiveData8(SPIx));
}

void Wait_SPI_Busy (void)
{
	//espero que se transfiera el dato
	while ((SPIx->SR & SPI_BSY) != 0);
}
void Send_SPI_Multiple (unsigned char a)
{
	//espero que haya lugar en el buffer
	while ((SPIx->SR & SPI_TXE) == 0);

	//*(__IO uint8_t *) SPIx->DR = a;
	SPI_SendData8(SPIx, a);

}

void Send_SPI_Single (unsigned char a)
{
	//espero que se libere el buffer
	while ((SPIx->SR & SPI_TXE) == 0);

	//tengo espacio
	//SPIx->DR = a;
	SPI_SendData8(SPIx, a);

	//espero que se transfiera el dato
	while ((SPIx->SR & SPI_BSY) != 0);
}

unsigned char Receive_SPI_Single (void)
{
	unsigned char dummy;

	//espero que se libere el buffer
	while (((SPIx->SR & SPI_TXE) == 0) || ((SPIx->SR & SPI_BSY) != 0));

	//limpio buffer RxFIFO
	while ((SPIx->SR & SPI_RXNE) != 0)
		dummy = SPIx->DR;

	//SPIx->DR = 0xff;
	SPI_SendData8(SPIx, 0xff);


	//espero que se transfiera el dato
	while ((SPIx->SR & SPI_BSY) != 0);

	dummy = (unsigned char) SPIx->DR;
	return dummy;
}
