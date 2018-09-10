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

//----------- Defines For Configuration --------------//
//----------- Some SPI Configurations ----------------//
//----------- End of SPI Configurations --------------//

//--- Exported constants ---//
#define SPI_RXNE	0x0001
#define SPI_TXE		0x0002
#define SPI_BSY		0x0080
#define SPI_OVR		0x0040

//Clock Peripherals
#define RCC_SPI1_CLK 			(RCC->APB2ENR & 0x00001000)
#define RCC_SPI1_CLK_ON 		RCC->APB2ENR |= 0x00001000
#define RCC_SPI1_CLK_OFF		RCC->APB2ENR &= ~0x00001000


//--- Exported Module Functions ------------
void SPI_Config(void);
void Send_SPI (unsigned char * , unsigned char);
unsigned char Send_Receive_SPI (unsigned char a);
void Wait_SPI_Busy (void);
void Send_SPI_Multiple (unsigned char);
void Send_SPI_Single (unsigned char);
unsigned char Receive_SPI_Single (void);

#endif /* _SPI_H_ */
