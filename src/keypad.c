//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### KEYPAD.C #######################################
//------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "keypad.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "spi.h"
#include "display_7seg.h"

// #include <string.h>
// #include <stdio.h>

#include "flash_program.h"    //TODO: sacar esto tambien!!!

/* Externals variables ---------------------------------------------------------*/
//para el timer
extern volatile unsigned char keypad_timeout;
extern volatile unsigned short keypad_interdigit_timeout;

#ifdef CONFIGURATION_IN_SST    //TODO: sacra esto a la mierda
extern parameters_typedef param_struct;
#endif


/* Externals functions??? ------------------------------------------------------*/
extern void BuzzerCommands(unsigned char, unsigned char);


/* Global variables ------------------------------------------------------------*/
unsigned char last_keypad_key = 0;
unsigned char keypad_state = 0;

// display_sm_t display_state = DISPLAY_SM_INIT;

/* Private Module Functions ----------------------------------------------------*/
unsigned char ReadSwitches_Internals (void);

/* Module Functions ------------------------------------------------------------*/

//funciona tipo Zero Order Hold con tiempo de muestreo KEYPAD_TIMEOUT
//revisa los switches cada 50ms, le da tiempo al display para mostrar los numeros
//devuelve el resultado de ReadSwitches_Internal(); 1 a 12 para keys o 0 para ninguna
unsigned char UpdateSwitches (void)
{
    if (!keypad_timeout)
    {
        last_keypad_key = ReadSwitches_Internals ();
        Display_ShowNumbersAgain();
        keypad_timeout = KEYPAD_TIMEOUT;
    }
    return last_keypad_key;
}

//se la llama para conocer la operacion de keypad seleccionada
//devuelve keypad_state KNUMBER_FINISH
//devuelve punteros a cada tecla en orden o posicion decimal con las 3 teclas combinadas
unsigned char CheckKeypad (unsigned char * sp0, unsigned char * sp1, unsigned char * sp2, unsigned short * posi)
{
    unsigned char switches = 0;

    switches = UpdateSwitches();

    switch (keypad_state)
    {
    case KNONE:

        if ((switches != NO_KEY) && (switches != STAR_KEY) && (switches != POUND_KEY))
        {
            //se presiono un numero voy a modo grabar codigo
            //reviso si fue 0
            if (switches == ZERO_KEY)
            {
                *sp0 = 0;
                Display_ShowNumbers(DISPLAY_ZERO);
            }
            else
            {
                *sp0 = switches;
                Display_ShowNumbers(switches);
            }
            BuzzerCommands(BUZZER_SHORT_CMD, 1);
            *sp1 = 0;
            *sp2 = 0;
            keypad_interdigit_timeout = param_struct.interdigit;
            keypad_state = KRECEIVING_A;
        }

        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;
        
        break;

    case KRECEIVING_A:
        //para validar switch anterior necesito que lo liberen
        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            keypad_state = KRECEIVING_B;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_B:			//segundo digito o confirmacion del primero
        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;            

        if (((switches > NO_KEY) && (switches < 10)) || (switches == ZERO_KEY))	//es un numero 1 a 9 o 0
        {
            if (switches == ZERO_KEY)
            {
                *sp1 = 0;
                Display_ShowNumbers(DISPLAY_ZERO);
            }
            else
            {
                *sp1 = switches;
                Display_ShowNumbers(switches);
            }
            BuzzerCommands(BUZZER_SHORT_CMD, 1);
            keypad_state = KRECEIVING_C;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        //si esta apurado un solo numero
        if (switches == POUND_KEY)
        {
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            *sp2 = *sp0;
            *sp0 = 0;
            *posi = *sp2;

            keypad_state = KNUMBER_FINISH;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_C:
        //para validar switch anterior necesito que lo liberen
        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            keypad_state = KRECEIVING_D;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_D:				//tercer digito o confirmacion del segundo
        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;

        if (((switches > NO_KEY) && (switches < 10)) || (switches == ZERO_KEY))	//es un numero 1 a 9 o 0
        {
            if (switches == ZERO_KEY)
            {
                *sp2 = 0;
                Display_ShowNumbers(DISPLAY_ZERO);
            }
            else
            {
                *sp2 = switches;
                Display_ShowNumbers(switches);
            }

            BuzzerCommands(BUZZER_SHORT_CMD, 1);
            keypad_state = KRECEIVING_E;
            keypad_interdigit_timeout = param_struct.interdigit;
        }

        //si esta apurado dos numeros
        if (switches == POUND_KEY)
        {
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            *sp2 = *sp0;
            *posi = *sp2 * 10 + *sp1;

            keypad_state = KNUMBER_FINISH;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_E:
        //para validar switch anterior necesito que lo liberen
        if (switches == NO_KEY)
        {
            Display_ShowNumbers(DISPLAY_NONE);
            keypad_state = KRECEIVING_F;
            keypad_interdigit_timeout = param_struct.interdigit;

        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;

        break;

    case KRECEIVING_F:
        if (switches == STAR_KEY)
            keypad_state = KCANCELLING;

        if (switches == POUND_KEY)	//es la confirmacion
        {
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            *posi = *sp0 * 100 + *sp1 * 10 + *sp2;

            keypad_state = KNUMBER_FINISH;
        }

        if (!keypad_interdigit_timeout)
            keypad_state = KTIMEOUT;
        break;

    case KCANCELLING:
            //se cancelo la operacion
            Display_ShowNumbers(DISPLAY_NONE);
            BuzzerCommands(BUZZER_HALF_CMD, 1);
            keypad_state = KCANCEL;
            
        break;
        
    case KNUMBER_FINISH:
    case KCANCEL:
    case KTIMEOUT:
    default:
        keypad_state = KNONE;
        break;
    }

    return keypad_state;
}

//devuelve el nuemero de key presionado entre 1 y 12
//devuelve 0 si no habia ninguna tecla activa
unsigned char ReadSwitches_Internals (void)
{
    unsigned short sw = 0;
    unsigned char sw0 = 0;
    unsigned char sw1 = 0;

    //bajo la velocidad del SPI
    SPI1->CR1 &= ~SPI_CR1_SPE;		//deshabilito periferico
    //clk / 256;
    SPI1->CR1 &= 0xFFC7;
    SPI1->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2;

    SPI1->CR1 |= SPI_CR1_SPE;		//habilito periferico

    OE_OFF;
    PS_ON;

    sw0 = SPI_Receive_Single();
    sw1 = SPI_Receive_Single();

    PS_OFF;
    OE_ON;

    sw = sw1;
    sw <<= 8;
    sw |= sw0;

    switch (sw)
    {
    case 0xBFBE:
        sw = 1;
        break;

    case 0xBFBD:
        sw = 2;
        break;

    case 0xBFBB:
        sw = 3;
        break;

    case 0xBFB7:
        sw = 4;
        break;

    case 0xBFAF:
        sw = 5;
        break;

    case 0xBF9F:
        sw = 6;
        break;

    case 0xBEBF:
        sw = 7;
        break;

    case 0xBDBF:
        sw = 8;
        break;

    case 0xBBBF:
        sw = 9;
        break;

    case 0xB7BF:
        sw = 10;
        break;

    case 0xAFBF:
        sw = 11;
        break;

    case 0x9FBF:
        sw = 12;
        break;

    default:
        sw = 0;
        break;
    }

    //subo la velocidad del SPI
    SPI1->CR1 &= ~SPI_CR1_SPE;		//deshabilito periferico
    //clk / 8;
    SPI1->CR1 &= 0xFFC7;
    SPI1->CR1 |= SPI_CR1_BR_1;

    SPI1->CR1 |= SPI_CR1_SPE;		//habilito periferico

    return (unsigned char) sw;
}

//--- end of file ---//
