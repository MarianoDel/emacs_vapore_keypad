//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### SPI.C #################################
//---------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "stm32f0xx.h"



/* Externals ------------------------------------------------------------------*/

/* Globals --------------------------------------------------------------------*/


/* Module Functions -----------------------------------------------------------*/
void SPI_Config(void)
{
    unsigned long temp;

    //funciones alternativas de los pines PB3 PB4 PB5
    temp = GPIOB->AFR[0];
    temp &= 0xFF000FFF;
    temp |= 0x00000000;	//PB5 -> AF0; PB4 -> AF0; PB3 -> AF0;
    GPIOB->AFR[0] = temp;

    
    //Habilitar Clk
    if (!RCC_SPI1_CLK)
        RCC_SPI1_CLK_ON;

    //Configuracion SPI
    SPI1->CR1 = 0;
    //SPI speed; clk / 8; master
    SPI1->CR1 |= SPI_CR1_BR_1;
    //CPOL High; CPHA second clock
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM | SPI_CR1_SSI;

    //thresh 8 bits; data 8 bits;
    SPI1->CR2 = SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;

    SPI1->CR1 |= SPI_CR1_SPE;		//habilito periferico
}

unsigned char Send_Receive_SPI (unsigned char a)
{
	unsigned char rx;
	unsigned char j;

	//primero limpio buffer rx spi
	while ((SPI1->SR & SPI_SR_RXNE) == 1)
	{
		rx = SPI1->DR & 0x0F;
	}

	//espero que haya lugar en el buffer
	while ((SPI1->SR & SPI_TXE) == 0);

	*(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = a; //evito enviar 16bits problemas de compilador

    //espero tener el dato en RX
    for (j = 0; j < 150; j++)
    {
    	asm("nop");
    }

    return (SPI1->DR & 0x0F);
}

void Wait_SPI_Busy (void)
{
	//espero que se transfiera el dato
	while ((SPI1->SR & SPI_BSY) != 0);
}

void Send_SPI_Multiple (unsigned char a)
{
	//espero que haya lugar en el buffer
	while ((SPI1->SR & SPI_TXE) == 0);

	//*(__IO uint8_t *) SPIx->DR = a;
	*(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = a; //evito enviar 16bits problemas de compilador

}

void Send_SPI_Single (unsigned char a)
{
	//espero que se libere el buffer
	while ((SPI1->SR & SPI_TXE) == 0);

	//tengo espacio
	//SPIx->DR = a;
	//SPIx->DR = a;
	*(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = a; //evito enviar 16bits problemas de compilador

	//espero que se transfiera el dato
	while ((SPI1->SR & SPI_BSY) != 0);
}

unsigned char Receive_SPI_Single (void)
{
	unsigned char dummy;

	//espero que se libere el buffer
	while (((SPI1->SR & SPI_TXE) == 0) || ((SPI1->SR & SPI_BSY) != 0));

	//limpio buffer RxFIFO
	while ((SPI1->SR & SPI_RXNE) != 0)
		dummy = SPI1->DR;

	*(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = 0xff; //evito enviar 16bits problemas de compilador

	//espero que se transfiera el dato
	while ((SPI1->SR & SPI_BSY) != 0);

	dummy = (unsigned char) SPI1->DR;
	return dummy;
}

//--- end of file ---//
