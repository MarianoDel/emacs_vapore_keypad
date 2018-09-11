//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### USART.H ################################
//---------------------------------------------
#ifndef _USART_H_
#define _USART_H_

//----------- Defines For Configuration --------------//
//----------- Some USART Configurations ----------------//
#define SIZEOF_DATA    128
#define BUFFRX_DIM SIZEOF_DATA
#define BUFFTX_DIM SIZEOF_DATA

//----------- End of USART Configurations --------------//

//--- Exported types ---//
//--- Exported constants ---//
#define USART_9600		5000
#define USART_115200		416
#define USART_250000		192

//--- Exported macro ---//
#define USART1_CLK (RCC->APB2ENR & 0x00004000)
#define USART1_CLK_ON RCC->APB2ENR |= 0x00004000
#define USART1_CLK_OFF RCC->APB2ENR &= ~0x00004000

#define USART1_RX_DISA	USART1->CR1 &= 0xfffffffb
#define USART1_RX_ENA	USART1->CR1 |= 0x04


#define USARTx                           USART1
#define USARTx_CLK                       RCC_APB2Periph_USART1
#define USARTx_APBPERIPHCLOCK            RCC_APB2PeriphClockCmd
#define USARTx_IRQn                      USART1_IRQn
#define USARTx_IRQHandler                USART1_IRQHandler


//en PA9 y PA10
#define USARTx_TX_PIN                    GPIO_Pin_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define USARTx_TX_SOURCE                 GPIO_PinSource9
#define USARTx_TX_AF                     GPIO_AF_1

#define USARTx_RX_PIN                    GPIO_Pin_10
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define USARTx_RX_SOURCE                 GPIO_PinSource10
#define USARTx_RX_AF                     GPIO_AF_1

/*
//en PB6 y PB7
#define USARTx_TX_PIN                    GPIO_Pin_6
#define USARTx_TX_GPIO_PORT              GPIOB
#define USARTx_TX_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define USARTx_TX_SOURCE                 GPIO_PinSource6
#define USARTx_TX_AF                     GPIO_AF_0

#define USARTx_RX_PIN                    GPIO_Pin_7
#define USARTx_RX_GPIO_PORT              GPIOB
#define USARTx_RX_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define USARTx_RX_SOURCE                 GPIO_PinSource7
#define USARTx_RX_AF                     GPIO_AF_0
*/


//--- Exported Module Functions ------------
unsigned char UpdateUart(unsigned char);
void USART_Config(void);
//void USARTx_receive (void);
unsigned char USARTx_Send(char *);
void Usart_Time_1ms (void);
void USART1_IRQHandler(void);
void Usart1SendUnsigned(unsigned char *, unsigned char);
void Usart1Send (char *);
void Usart1Config (void);

#endif /* _USART_H_ */

//--- end of file ---//

