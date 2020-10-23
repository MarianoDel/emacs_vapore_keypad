//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### SPI.C #################################
//---------------------------------------------
#include "spi.h"
#include "stm32f0xx.h"


/* Externals ------------------------------------------------------------------*/

/* Globals --------------------------------------------------------------------*/


/* Module Functions -----------------------------------------------------------*/
void SPI_Config(void)
{
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

    //funciones alternativas de los pines PB3 PB4 PB5
    unsigned int temp;
    temp = GPIOB->AFR[0];
    temp &= 0xFF000FFF;
    temp |= 0x00000000;	//PB5 -> AF0; PB4 -> AF0; PB3 -> AF0;
    GPIOB->AFR[0] = temp;

    SPI1->CR1 |= SPI_CR1_SPE;		//habilito periferico
}


unsigned char SPI_Send_Receive (unsigned char a)
{
    unsigned char dummy;

    //primero limpio buffer rx spi
    while ((SPI1->SR & SPI_SR_RXNE) == 1)
    {
        dummy = SPI1->DR & 0x0F;
    }

    //espero que haya lugar en el buffer
    while ((SPI1->SR & SPI_SR_TXE) == 0);

    *(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = a; //evito enviar 16bits problemas de compilador

    //espero tener el dato en RX
    for (unsigned char j = 0; j < 150; j++)
    {
    	asm("nop");
    }

    dummy = SPI1->DR & 0x0F;
    return dummy;
}


void SPI_Busy_Wait (void)
{
    //espero que se transfiera el dato
    while ((SPI1->SR & SPI_SR_BSY) != 0);
}


void SPI_Send_Multiple (unsigned char a)
{
    //espero que haya lugar en el buffer
    while ((SPI1->SR & SPI_SR_TXE) == 0);

    //*(__IO uint8_t *) SPI1->DR = a;
    *(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = a; //evito enviar 16bits problemas de compilador

}


void SPI_Send_Single (unsigned char a)
{
    //espero que se libere el buffer
    while ((SPI1->SR & SPI_SR_TXE) == 0);

    //tengo espacio
    //SPI1->DR = a;
    //SPI1->DR = a;
    *(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = a; //evito enviar 16bits problemas de compilador

    //espero que se transfiera el dato
    while ((SPI1->SR & SPI_SR_BSY) != 0);
}


unsigned char SPI_Receive_Single (void)
{
    unsigned char dummy;

    //espero que se libere el buffer
    while (((SPI1->SR & SPI_SR_TXE) == 0) || ((SPI1->SR & SPI_SR_BSY) != 0));

    //limpio buffer RxFIFO
    while ((SPI1->SR & SPI_SR_RXNE) != 0)
        dummy = SPI1->DR;

    *(__IO uint8_t *) ((uint32_t)SPI1 + (uint32_t)0x0C) = 0xff; //evito enviar 16bits problemas de compilador

    //espero que se transfiera el dato
    while ((SPI1->SR & SPI_SR_BSY) != 0);

    dummy = (unsigned char) SPI1->DR;
    return dummy;
}

//--- end of file ---//
