//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### RWS317.C #######################################
//------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "rws317.h"
#include "hard.h"
#include "flash_program.h"
#include "sst25codes.h"

//desde que saque main.h
#include "stm32f0xx.h"
#include <stdio.h>
#include "usart.h"
//desde que saque main.h


/* Externals variables ---------------------------------------------------------*/
extern volatile unsigned short code0;
extern volatile unsigned short code1;
extern volatile unsigned char errorcode;
extern volatile unsigned char pilot_code;
extern unsigned int * pmem;

/* Global variables ------------------------------------------------------------*/
volatile unsigned char ctrol_new = 0;
volatile unsigned char bitcount = 0;
volatile unsigned char bitstate = 0;
volatile unsigned short lastC0V = 0;
volatile unsigned char no_silent = 0;

volatile unsigned char on_receipt = 0;

unsigned char code_state = 0;


/* Module Functions ------------------------------------------------------------*/
//0 si hay algun error de 1 a 4 los botones
unsigned char CheckForButtons(unsigned short * p_posi, unsigned int * p_code)
{
    unsigned char resp = 0;

    if (RxCode() == ENDED_OK)
    {
        *p_code = code0;
        *p_code <<= 16;
        *p_code |= code1;

        *p_posi = CheckBaseCodeInMemory(*p_code);

        if ((*p_posi >= 0) && (*p_posi <= 1023))	//el codigo existe en memoria
            resp = CheckButtonInCode(*p_code);		//reviso el boton
    }
    return resp;
}


//contesta entre 0 y 1023 si existe en memoria, sino contesta 0xFFFF
unsigned short CheckCodeInMemory_FLASH (unsigned int code_to_find)
{
	unsigned short i;

	for (i = 0; i < FLASH_PAGE_SIZE; i++)
	{
		if (code_to_find == *(pmem + i))
			return i;
	}

	return 0xffff;
}

//contesta entre 0 y 1023 si existe en memoria, sino contesta 0xFFFF
unsigned short CheckBaseCodeInMemory_FLASH (unsigned int code_to_find)
{
	unsigned short i;

	//code_to_find = code_to_find & 0xFFFFFFCF;
	//code_to_find = code_to_find & 0xFFFFFFFC;
	code_to_find = code_to_find & 0xFFFFFF0F;

	for (i = 0; i < FLASH_PAGE_SIZE; i++)
	{
		//if (code_to_find == (*(pmem + i) & 0xFFFFFFCF))
		//if (code_to_find == (*(pmem + i) & 0xFFFFFFFC))
		if (code_to_find == (*(pmem + i) & 0xFFFFFF0F))
			return i;
	}

	return 0xffff;
}


unsigned int CheckIndexInMemory_FLASH (unsigned short index_to_find)
{
	if (index_to_find < FLASH_PAGE_SIZE)
		return *(pmem + index_to_find);
	else
		return 0xffffffff;
}

//0 sino existe el boton sino de 1 a 4
unsigned char CheckButtonRemote (unsigned short c0,  unsigned short c1)
{
	if (c0 != 0x5554)
		return REM_NO;

	if (c1 == 0x0206)
		return REM_B1;

	if (c1 == 0x0218)
		return REM_B2;

	if (c1 == 0x021E)
		return REM_B3;

	if (c1 == 0x0260)
		return REM_B4;

	if (c1 == 0x0266)
		return REM_B5;

	if (c1 == 0x0278)
		return REM_B6;

	if (c1 == 0x027E)
		return REM_B7;

	if (c1 == 0x0380)
		return REM_B8;

	if (c1 == 0x0386)
		return REM_B9;

	if (c1 == 0x0398)
		return REM_B10;

	if (c1 == 0x039E)
		return REM_B11;

	if (c1 == 0x03E0)
		return REM_B12;

	return REM_NO;
}

unsigned char RxCode (void)
{
	char str[30];

	switch (code_state)
	{
		case START:
			//destrabo la interrupcion y espero silencio
			//CodeInterruptEna();
			no_silent = 0;
			bitstate = WAIT_FOR_SILENT;
			errorcode = ERR_CONTINUE;
			pilot_code = PILOT_CODE_MS;		//el timer descuenta directamente
//			F5PLUS_ON;
			code_state++;
			break;

		case SEARCHING:
			if (no_silent == 0)
			{
				if (pilot_code == 0)
				{
					//se silencio durante PILOT_CODE_MS debo estar por recibir codigo
					code_state++;
					//destrabo la interrupcion
					bitstate = FIRST_HIGH;
//					F5PLUS_OFF;
					TIM14->CNT = 0;
				}
			}
			else
				code_state = START;
				//code_state = START_B;
			break;

		case RECEIVING:
			//reviso si termino el codigo (bueno o malo)
			if (errorcode != ERR_CONTINUE)
			{
				//termino el codigo
				code_state++;
			}
			break;

		case ENDED:
			if (errorcode == ERR_FINISH_OK)
			{
//				F5PLUS_OFF;
				sprintf(str, "Ended OK: %04X %04X\r\n", code0, code1);
				Usart1Send(str);
				code_state = ENDED_OK;

				//si tengo un codigo bueno trabo la interrrupcion
				//CodeInterruptDisa();
			}
			else
			{
				code_state = START;
#ifdef DEBUG_ERROR_CODES
				 if (errorcode == ERR_FIRST_HIGH)
				 {
					 Usart1Send((char *)"Error First High\r\n");
				 }
				 else if (errorcode == ERR_BIT_NEXT)
				 {
					 sprintf(str, "Error in bit: %d\r\n", bitcount);
					 Usart1Send(str);
				 }
				 else
				 {
					 Usart1Send((char *)"Error other??\r\n");
				 }
#endif
			}
			break;

		case ENDED_OK:
			code_state = START;
			break;

		default:
			code_state = START;
			break;
	}
	return code_state;
}

void Timer_Interrupt_Handler (unsigned short tpm1)
{
//	char str [20];
	unsigned short duration = 0;

	if (lastC0V > tpm1)
		duration = (0xffff - lastC0V) + tpm1;			//dio la vuelta
	else
		duration = tpm1 - lastC0V;	//lo normal

	lastC0V = tpm1;

//	if (on_receipt)
//	{
//		if (F5PLUS)
//			F5PLUS_OFF;
//		else
//			F5PLUS_ON;
//	}


#ifdef WITHOUT_ABSOLUTE_VALUE_CTROL_NEW
	switch (bitstate)
	{
		case WAIT_FOR_SILENT:
			no_silent++;
			break;

		case FIRST_HIGH:
			bitcount = 0;
			code0 = 0;
			code1 = 0;
			bitstate++;
			break;

		case FIRST_LOW:
			if ((duration > PILOT_MIN_NEW) && (duration < PILOT_MAX_NEW))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate++;
				ctrol_new = 1;
				Usart1Send((char *)"new ");
			}
			else
			{
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_FIRST_HIGH;
			}
			break;

		case NEXT:
			if ((duration > LAMBDA1_FROM0_MIN_NEW) && (duration < LAMBDA1_FROM0_MAX_NEW))
			{
				//puede ser un 0
				bitstate = CHECK0;
			}
			else if ((duration > LAMBDA2_FROM1_MIN_NEW) && (duration < LAMBDA2_FROM1_MAX_NEW))
			{
				//puede ser un 1
				bitstate = CHECK1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
			}
			break;

		case CHECK0:
			if ((duration > LAMBDA2_FROM0_MIN_NEW) && (duration < LAMBDA2_FROM0_MAX_NEW))
			{
				//llego el 0
				bitstate = NEXT;
				//si no es el primer bit
				if (bitcount)
				{
					if (bitcount < 16)
						code0 <<= 1;
					else
						code1 <<= 1;
				}
				bitcount++;

			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_0;
			}
			break;

		case CHECK1:
			if ((duration > LAMBDA1_FROM1_MIN_NEW) && (duration < LAMBDA1_FROM1_MAX_NEW))
			{
				//llego el 1
				bitstate = NEXT;
				//si no es el primer bit
				if (bitcount)
				{
					if (bitcount < 16)
					{
						code0 += 1;
						code0 <<= 1;
					}
					else
					{
						code1 += 1;
						code1 <<= 1;
					}
				}
				bitcount++;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_1;
			}
			break;

		case CODE_FINISH:

			break;

		default:		//ver si no tengo que ir directo a NEXT
			bitstate = WAIT_FOR_SILENT;
			break;
	}
	//me fijo si termino el paquete

	if (bitcount >= 28)
	{
		bitstate = CODE_FINISH;
		errorcode = ERR_FINISH_OK;
	}

#endif

#ifdef WITHOUT_ABSOLUTE_VALUE_CTROL_EV1527_AND_PT2264
	switch (bitstate)
	{
		case WAIT_FOR_SILENT:
			no_silent++;
			break;

		case FIRST_HIGH:
			bitcount = 0;
			code0 = 0;
			code1 = 0;
			bitstate++;
			break;

		case FIRST_LOW:
			if ((duration > ALPHA1_MIN) && (duration < ALPHA1_MAX))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate = CHECK0;
				on_receipt = 1;
				Usart1Send((char *)"ev1527_0");
			}
			else if ((duration > ALPHA3_MIN) && (duration < ALPHA3_MAX))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate = CHECK1;
				on_receipt = 1;
				Usart1Send((char *)"ev1527_1");
			}
			else
			{
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_FIRST_HIGH;
				on_receipt = 0;
			}
			break;

		case NEXT:
			if ((duration > ALPHA1_MIN) && (duration < ALPHA1_MAX))
			{
				//puede ser un 0
				bitstate = CHECK0;
			}
			else if ((duration > ALPHA3_MIN) && (duration < ALPHA3_MAX))
			{
				//puede ser un 1
				bitstate = CHECK1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
				on_receipt = 0;
			}
			break;

		case CHECK0:
			if ((duration > ALPHA3_MIN) && (duration < ALPHA3_MAX))
			{
				//llego el 0
				bitstate = NEXT;
				bitcount++;

				if (bitcount < 16)
					code0 <<= 1;
				else
					code1 <<= 1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_0;
				on_receipt = 0;
			}
			break;

		case CHECK1:
			if ((duration > ALPHA1_MIN) && (duration < ALPHA1_MAX))
			{
				//llego el 1
				bitstate = NEXT;
				bitcount++;
				if (bitcount < 16)
				{
					code0 += 1;
					code0 <<= 1;
				}
				else
				{
					code1 += 1;
					code1 <<= 1;
				}
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_1;
				on_receipt = 0;
			}
			break;

		case CODE_FINISH:

			break;

		default:		//ver si no tengo que ir directo a NEXT
			bitstate = WAIT_FOR_SILENT;
			break;
	}
	//me fijo si termino el paquete

	if (bitcount >= 24)
	{
		bitstate = CODE_FINISH;
		errorcode = ERR_FINISH_OK;
	}

#endif

#ifdef WITHOUT_ABSOLUTE_VALUE_CTROL_EV1527
	switch (bitstate)
	{
		case WAIT_FOR_SILENT:
			no_silent++;
			break;

		case FIRST_HIGH:
			bitcount = 0;
			code0 = 0;
			code1 = 0;
			bitstate++;
			break;

		case FIRST_LOW:
			if ((duration > PILOT_MIN0_NEW) && (duration < PILOT_MAX0_NEW))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate = CHECK0;
				ctrol_new = 1;
				Usart1Send((char *)"ev1527_0");
			}
			else if ((duration > PILOT_MIN1_NEW) && (duration < PILOT_MAX1_NEW))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate = CHECK1;
				ctrol_new = 1;
				Usart1Send((char *)"ev1527_1");
			}
			else
			{
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_FIRST_HIGH;
			}
			break;

		case NEXT:
			if ((duration > LAMBDA1_FROM0_MIN_NEW) && (duration < LAMBDA1_FROM0_MAX_NEW))
			{
				//puede ser un 0
				bitstate = CHECK0;
			}
			else if ((duration > LAMBDA2_FROM1_MIN_NEW) && (duration < LAMBDA2_FROM1_MAX_NEW))
			{
				//puede ser un 1
				bitstate = CHECK1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
			}
			break;

		case CHECK0:
			if ((duration > LAMBDA2_FROM0_MIN_NEW) && (duration < LAMBDA2_FROM0_MAX_NEW))
			{
				//llego el 0
				bitstate = NEXT;
				bitcount++;

				if (bitcount < 16)
					code0 <<= 1;
				else
					code1 <<= 1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_0;
			}
			break;

		case CHECK1:
			if ((duration > LAMBDA1_FROM1_MIN_NEW) && (duration < LAMBDA1_FROM1_MAX_NEW))
			{
				//llego el 1
				bitstate = NEXT;
				bitcount++;
				if (bitcount < 16)
				{
					code0 += 1;
					code0 <<= 1;
				}
				else
				{
					code1 += 1;
					code1 <<= 1;
				}
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_1;
			}
			break;

		case CODE_FINISH:

			break;

		default:		//ver si no tengo que ir directo a NEXT
			bitstate = WAIT_FOR_SILENT;
			break;
	}
	//me fijo si termino el paquete

	if (bitcount >= 24)
	{
		bitstate = CODE_FINISH;
		errorcode = ERR_FINISH_OK;
	}

#endif

#ifdef WITHOUT_ABSOLUTE_VALUE_CTROL_HT6P20B2_AND_PT2264
	switch (bitstate)
	{
		case WAIT_FOR_SILENT:
			no_silent++;
			break;

		case FIRST_HIGH:
			bitcount = 0;
			code0 = 0;
			code1 = 0;
			bitstate++;
			break;

		case FIRST_LOW:
			//veo si puedo establecer directamente que control es
			if ((duration > PILOT_HT_MIN) && (duration < ALPHA1_MIN))
			{
				//seguro es HT6P20B2
				bitstate = NEXT_HT;
				on_receipt = 1;
				ctrol_new = CTRL_HT;
				Usart1Send((char *)"ht6__");
			}
			else if ((duration >= ALPHA1_MIN) && (duration < PILOT_HT_MAX))
			{
				//puede ser HT6P20B2 o PT2264
				on_receipt = 1;
				bitstate = NEXT_ALL;
			}
			else if ((duration >= PILOT_HT_MAX) && (duration < ALPHA1_MAX))
			{
				//seguro es PT2264
				bitstate = CHECK0_PT;
				on_receipt = 1;
				ctrol_new = CTRL_PT;
				Usart1Send((char *)"pt2264__");
			}
			else
			{
				bitstate = FIRST_HIGH;
				errorcode = ERR_FIRST_HIGH;
				on_receipt = 0;
			}
			break;

		case NEXT_ALL:	//tengo todavia que definir que tipo de control es
			if ((duration > LAMBDA1_MIN) && (duration < LAMBDA1_MAX))
			{
				//puede ser un 0
				bitstate = CHECK0_HT;
				ctrol_new = CTRL_HT;
				Usart1Send((char *)"ht6_0");
			}
			else if ((duration > LAMBDA2_MIN) && (duration < LAMBDA2_MAX))
			{
				//puede ser un 1
				bitstate = CHECK1_HT;
				ctrol_new = CTRL_HT;
				Usart1Send((char *)"ht6_1");
			}
			else if ((duration > ALPHA3_MIN) && (duration < ALPHA3_MAX))
			{
				//llego el 0
				bitstate = NEXT_PT;
				ctrol_new = CTRL_PT;
				bitcount++;

				if (bitcount < 16)
					code0 <<= 1;
				else
					code1 <<= 1;
			}
			else
			{
				//error en bit
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
				on_receipt = 0;
			}
			break;

		case NEXT_PT:
			if ((duration > ALPHA1_MIN) && (duration < ALPHA1_MAX))
			{
				//puede ser un 0
				bitstate = CHECK0_PT;
			}
			else if ((duration > ALPHA3_MIN) && (duration < ALPHA3_MAX))
			{
				//puede ser un 1
				bitstate = CHECK1_PT;
			}
			else
			{
				//error en bit
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
				on_receipt = 0;
			}
			break;

		case CHECK0_PT:
			if ((duration > ALPHA3_MIN) && (duration < ALPHA3_MAX))
			{
				//llego el 0
				bitstate = NEXT_PT;
				bitcount++;

				if (bitcount < 16)
					code0 <<= 1;
				else
					code1 <<= 1;
			}
			else
			{
				//error en bit
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_0;
				on_receipt = 0;
			}
			break;

		case CHECK1_PT:
			if ((duration > ALPHA1_MIN) && (duration < ALPHA1_MAX))
			{
				//llego el 1
				bitstate = NEXT_PT;
				bitcount++;
				if (bitcount < 16)
				{
					code0 += 1;
					code0 <<= 1;
				}
				else
				{
					code1 += 1;
					code1 <<= 1;
				}
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_1;
				on_receipt = 0;
			}
			break;

		case NEXT_HT:
			if ((duration > LAMBDA1_MIN) && (duration < LAMBDA1_MAX))
			{
				//puede ser un 0
				bitstate = CHECK0_HT;
			}
			else if ((duration > LAMBDA2_MIN) && (duration < LAMBDA2_MAX))
			{
				//puede ser un 1
				bitstate = CHECK1_HT;
			}
			else
			{
				//error en bit
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
			}
			break;

		case CHECK0_HT:
			if ((duration > LAMBDA2_MIN) && (duration < LAMBDA2_MAX))
			{
				//llego el 0
				bitstate = NEXT_HT;
				//si no es el primer bit
				if (bitcount)
				{
					if (bitcount < 16)
						code0 <<= 1;
					else
						code1 <<= 1;
				}
				bitcount++;
			}
			else
			{
				//error en bit
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_0;
				on_receipt = 0;
			}
			break;

		case CHECK1_HT:
			if ((duration > LAMBDA1_MIN) && (duration < LAMBDA1_MAX))
			{
				//llego el 1
				bitstate = NEXT_HT;
				//si no es el primer bit
				if (bitcount)
				{
					if (bitcount < 16)
					{
						code0 += 1;
						code0 <<= 1;
					}
					else
					{
						code1 += 1;
						code1 <<= 1;
					}
				}
				bitcount++;
			}
			else
			{
				//error en bit
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_1;
				on_receipt = 0;
			}
			break;

		case CODE_FINISH:

			break;

		default:		//ver si no tengo que ir directo a NEXT
			bitstate = WAIT_FOR_SILENT;
			break;
	}
	//me fijo si termino el paquete

	if (ctrol_new == CTRL_HT)
	{
		if (bitcount >= 28)
		{
			bitstate = CODE_FINISH;
			errorcode = ERR_FINISH_OK;
			on_receipt = 0;
		}
	}
	else
	{
		//debe ser PT
		if (bitcount >= 24)
		{
			bitstate = CODE_FINISH;
			errorcode = ERR_FINISH_OK;
			on_receipt = 0;
		}
	}

#endif

#ifdef WITHOUT_ABSOLUTE_VALUE_CTROL_PT2264
	switch (bitstate)
	{
		case WAIT_FOR_SILENT:
			no_silent++;
			break;

		case FIRST_HIGH:
			bitcount = 0;
			code0 = 0;
			code1 = 0;
			bitstate++;
			break;

		case FIRST_LOW:
			if ((duration > PILOT_MIN0_NEW) && (duration < PILOT_MAX0_NEW))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate = CHECK0;
				ctrol_new = 1;
				on_receipt = 1;
				Usart1Send((char *)"pt2264_0");
			}
			else if ((duration > PILOT_MIN1_NEW) && (duration < PILOT_MAX1_NEW))	//tengo mi primer first high, ahora vienen los bits
			{
				bitstate = CHECK1;
				ctrol_new = 1;
				on_receipt = 1;
				Usart1Send((char *)"pt2264_1");
			}
			else
			{
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_FIRST_HIGH;
				on_receipt = 0;
			}
			break;

		case NEXT:
			if ((duration > LAMBDA1_FROM0_MIN_NEW) && (duration < LAMBDA1_FROM0_MAX_NEW))
			{
				//puede ser un 0
				bitstate = CHECK0;
			}
			else if ((duration > LAMBDA2_FROM1_MIN_NEW) && (duration < LAMBDA2_FROM1_MAX_NEW))
			{
				//puede ser un 1
				bitstate = CHECK1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_NEXT;
				on_receipt = 0;
			}
			break;

		case CHECK0:
			if ((duration > LAMBDA2_FROM0_MIN_NEW) && (duration < LAMBDA2_FROM0_MAX_NEW))
			{
				//llego el 0
				bitstate = NEXT;
				bitcount++;

				if (bitcount < 16)
					code0 <<= 1;
				else
					code1 <<= 1;
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_0;
				on_receipt = 0;
			}
			break;

		case CHECK1:
			if ((duration > LAMBDA1_FROM1_MIN_NEW) && (duration < LAMBDA1_FROM1_MAX_NEW))
			{
				//llego el 1
				bitstate = NEXT;
				bitcount++;
				if (bitcount < 16)
				{
					code0 += 1;
					code0 <<= 1;
				}
				else
				{
					code1 += 1;
					code1 <<= 1;
				}
			}
			else
			{
				//error en bit
				//bitstate = CODE_FINISH;
				bitstate = FIRST_HIGH;
				errorcode = ERR_BIT_1;
				on_receipt = 0;
			}
			break;

		case CODE_FINISH:

			break;

		default:		//ver si no tengo que ir directo a NEXT
			bitstate = WAIT_FOR_SILENT;
			break;
	}
	//me fijo si termino el paquete

	if (bitcount >= 24)
	{
		bitstate = CODE_FINISH;
		errorcode = ERR_FINISH_OK;
		on_receipt = 0;
	}

#endif

}

//--- end of file ---//
