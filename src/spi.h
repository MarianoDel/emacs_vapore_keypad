//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### SPI.H #################################
//---------------------------------------------
#ifndef _SPI_H_
#define _SPI_H_

//Clock Peripherals
#define RCC_SPI1_CLK    (RCC->APB2ENR & 0x00001000)
#define RCC_SPI1_CLK_ON    (RCC->APB2ENR |= 0x00001000)
#define RCC_SPI1_CLK_OFF    (RCC->APB2ENR &= ~0x00001000)


// Exported Module Functions ---------------------------------------------------
void SPI_Config(void);
unsigned char SPI_Send_Receive (unsigned char);
void SPI_Busy_Wait (void);
void SPI_Send_Multiple (unsigned char);
void SPI_Send_Single (unsigned char);
unsigned char SPI_Receive_Single (void);


#endif    /* _SPI_H_ */
