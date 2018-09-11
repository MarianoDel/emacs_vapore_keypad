//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### USART.C ################################
//---------------------------------------------
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "stm32f0xx.h"
#include "hard.h"
#include "comm.h"
#include <string.h>
#include "flash_program.h"


/* Externals ------------------------------------------------------------------*/
extern mem_bkp_typedef memory_backup;
extern volatile unsigned char binary_full;

extern volatile unsigned char usart1_have_data;
extern volatile unsigned char tx1buff[];
extern volatile unsigned char rx1buff[];


/* Globals ------------------------------------------------------------------*/
volatile unsigned short binary_bytes = 1024;

volatile unsigned char * ptx1;
volatile unsigned char * ptx1_pckt_index;
volatile unsigned char * prx1;


/* Module Functions -----------------------------------------------------------*/
//inline void UpdateUart(void)
unsigned char UpdateUart(unsigned char last_state)
{
    //TODO: mejorar esto revisando con usart_have_data
    //realizarla cpia de otraforma para no depender del llamado
    //a interpretar mensje desde aca

    //ya cambio bufrx_ready por usart_have_data

	if (usart1_have_data)            
	{
		usart1_have_data  = 0;
		last_state = InterpretarMsg (last_state, (char *) rx1buff);
		// memset((char *) rx1buff, '\0', sizeof(rx1buff));
		memset((char *) rx1buff, '\0', SIZEOF_DATA);
	}
	return last_state;
        
	// if (buffrx_ready)        
	// {
	// 	buffrx_ready  = 0;                
	// 	last_state = InterpretarMsg (last_state, (char *) buffrx);
	// 	memset(buffrx, '\0', sizeof(buffrx));
	// }
	// return last_state;        
}


void Usart1Config (void)
{
    unsigned long temp;

    if (!USART1_CLK)
        USART1_CLK_ON;

    ptx1 = tx1buff;
    ptx1_pckt_index = tx1buff;
    prx1 = rx1buff;

    USART1->BRR = USART_9600;
//	USART1->CR2 |= USART_CR2_STOP_1;	//2 bits stop
//	USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
//	USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;	//SIN TX
    USART1->CR1 = USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;	//para pruebas TX

    temp = GPIOA->AFR[1];
    temp &= 0xFFFFF00F;
    temp |= 0x00000110;	//PA10 -> AF1 PA9 -> AF1
    GPIOA->AFR[1] = temp;

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 5);
}

void USART1_IRQHandler(void)
{
    unsigned char dummy = 0;

    /* USART in Receiver Mode --------------------------------------------------*/
    if (USART1->ISR & USART_ISR_RXNE)
    {
        dummy = USART1->RDR & 0x0FF;

        //formato binario
        if (binary_bytes < 1024)
        {
            memory_backup.v_bkp_8u[binary_bytes] = dummy;
            binary_bytes++;
            if (binary_bytes == 1024)
                binary_full = 1;
        }
        else
        {
            if (prx1 < &rx1buff[SIZEOF_DATA])
            {
                if ((dummy == '\n') || (dummy == '\r') || (dummy == 26))		//26 es CTRL-Z
                {
                    *prx1 = '\0';
                    usart1_have_data = 1;
                }
                else
                {
                    *prx1 = dummy;
                    prx1++;
                }
            }
        }
    }
        
    /* USART in Transmitter Mode -------------------------------------------------*/
    if (USART1->CR1 & USART_CR1_TXEIE)
    {
        if (USART1->ISR & USART_ISR_TXE)
        {
            if ((ptx1 < &tx1buff[SIZEOF_DATA]) && (ptx1 < ptx1_pckt_index))
            {
                USART1->TDR = *ptx1;
                ptx1++;
            }
            else
            {
                ptx1 = tx1buff;
                ptx1_pckt_index = tx1buff;
                USART1->CR1 &= ~USART_CR1_TXEIE;
            }
        }
    }
        
    if ((USART1->ISR & USART_ISR_ORE) || (USART1->ISR & USART_ISR_NE) || (USART1->ISR & USART_ISR_FE))
    {
        USART1->ICR |= 0x0e;
        dummy = USART1->RDR;
    }
}

void Usart1Send (char * send)
{
    unsigned char i;

    i = strlen(send);
    Usart1SendUnsigned((unsigned char *) send, i);
}

void Usart1SendUnsigned(unsigned char * send, unsigned char size)
{
    if ((ptx1_pckt_index + size) < &tx1buff[SIZEOF_DATA])
    {
        memcpy((unsigned char *)ptx1_pckt_index, send, size);
        ptx1_pckt_index += size;
        USART1->CR1 |= USART_CR1_TXEIE;
    }
}

void Usart1SendSingle(unsigned char tosend)
{
    Usart1SendUnsigned(&tosend, 1);
}


//--- end of file ---//

