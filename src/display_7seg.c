//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DISPLAY_7SEG.C #######################################
//------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "display_7seg.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "spi.h"

#include <string.h>


/* Externals variables ---------------------------------------------------------*/
//para el timer
extern volatile unsigned short display_timeout;


/* Global variables ------------------------------------------------------------*/
unsigned char display_last_digit = 0;
char display_vector_numbers [LAST_NUMBER];
char * p_vector_numbers;

display_sm_t display_state = DISPLAY_SM_INIT;

/* Module Functions ------------------------------------------------------------*/
void SendSegment (unsigned char segment)
{
    OE_OFF;
    if (segment & 0x80)
    	Send_SPI_Single (0x01);
    else
    	Send_SPI_Single (0x00);

    segment <<= 1;
    Send_SPI_Single (segment);
    OE_ON;
}

void ShowNumbersAgain (void)
{
    ShowNumbers (display_last_digit);
}

//	dp g f e d c b a
//bits   7 6 5 4 3 2 1 0
//negados
const unsigned char v_display_numbers [] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
                                             0x80, 0x98, 0x7F, 0xBF, 0x9C, 0x06, 0xB0, 0xA3 };

//beware read the switches disables de displayed number
//system calls ShowNumbersAgain to solve this
void ShowNumbers (unsigned char number)	//from 0 to 9 are numbers; 10 to 15 are other symbols
{
    if ((number != DISPLAY_NONE) && (number < SIZEOF_VDISPLAY))
    {
        display_last_digit = number;
        number = v_display_numbers[number];
    }
    else
        number = 0xFF;

    
    //unsigned char a;
    //me fijo si lo saco con punto
    //if (number > 128)

    // number = number & 0x0F;
    // switch (number)
    // display_last_digit = number;
    // switch (number)
    // {
    // case 10:
    //     //number = 0x3F;
    //     number = 0xC0;
    //     break;

    // case DISPLAY_ERROR:
    //     //E
    //     number = 0x06;
    //     break;

    // case DISPLAY_ERROR2:
    //     //E inv
    //     number = 0xB0;
    //     break;

    // case DISPLAY_LINE:
    //     //raya media
    //     number = 0xBF;
    //     break;

    // case DISPLAY_REMOTE:
    //     //cuadro superior
    //     number = 0x9C;
    //     break;

    // case DISPLAY_SQR_DOWN:
    //     //cuadro inferior
    //     number = 0xA3;
    //     break;

    // case 1:
    //     //number = 0x06;
    //     number = 0xF9;
    //     break;
    // case 2:
    //     //number = 0x5B;
    //     number = 0xA4;
    //     break;
    // case 3:
    //     //number = 0x4F;
    //     number = 0xB0;
    //     break;
    // case 4:
    //     //number = 0x66;
    //     number = 0x99;
    //     break;
    // case 5:
    //     //number = 0x6D;
    //     number = 0x92;
    //     break;
    // case 6:
    //     //number = 0x7D;
    //     number = 0x82;
    //     break;
    // case 7:
    //     //number = 0x07;
    //     number = 0xF8;
    //     break;
    // case 8:
    //     //number = 0x7F;
    //     number = 0x80;
    //     break;
    // case 9:
    //     //number = 0x67;
    //     number = 0x98;
    //     break;
    // case 11:
    //     //number = 0x80;	//solo punto
    //     number = 0x7F;	//solo punto
    //     break;

    // default:
    //     //apagar el display
    //     number = 0xFF;
    //     break;
    // }
    SendSegment (number);
}

unsigned char DisplayIsFree (void)
{
    if (display_state == DISPLAY_SM_INIT)
        return 1;
    else
        return 0;
}

void UpdateDisplayResetSM (void)
{
    p_vector_numbers = display_vector_numbers;
    display_state = DISPLAY_SM_INIT;
}

void UpdateDisplayStartSM (void)
{
    display_state = DISPLAY_SM_SENDING;
}

//show secuences of numbers by calling to ShowNumbers
//the numbers must be setted by calling to VectorToDisplay()
//ONLY Numbers and the POINT
void UpdateDisplaySM (void)
{

    switch (display_state)
    {
    case DISPLAY_SM_INIT:
        break;

    case DISPLAY_SM_SENDING:
        if (*p_vector_numbers == '.')
            ShowNumbers(DISPLAY_POINT);
        else
            ShowNumbers(*p_vector_numbers - '0');
        
        p_vector_numbers++;
        display_state++;
        display_timeout = DISPLAY_TIMER_IN_ON;
        break;

    case DISPLAY_SM_SHOWING:
        if (!display_timeout)
        {
            ShowNumbers(DISPLAY_NONE);
            display_timeout = DISPLAY_TIMER_IN_OFF;
            display_state++;
        }
        break;

    case DISPLAY_SM_WAITING:
        if (!display_timeout)
        {
            if ((*p_vector_numbers != '\0') &&
                (p_vector_numbers < &display_vector_numbers[SIZEOF_VDISPLAY]))
                display_state = DISPLAY_SM_SENDING;
            else
            {
                //llegue al final
                p_vector_numbers = display_vector_numbers;
                // memset (numbers, '\0', sizeof(numbers));
                display_state = DISPLAY_SM_INIT;
            }
        }
        break;

    default:
        display_state = DISPLAY_SM_INIT;
        break;
    }
}

//carga los numeros a mostrar en secuencia en un vector
//del 1 al 9; 10 es cero; 11 es punto; 0, 12, 13, 14, 15 apagar
// void VectorToDisplay (unsigned char new_number)
// {
//     unsigned char i;
//     //me fijo si hay espacio
//     if (p_vector_numbers < &display_vector_numbers [LAST_NUMBER])
//     {
//         //busco la primer posicion vacia y pongo el nuevo numero
//         for (i = 0; i < LAST_NUMBER; i++)
//         {
//             if (display_vector_numbers[i] == 0)
//             {
//                 display_vector_numbers[i] = new_number;
//                 i = LAST_NUMBER;
//             }
//         }
//     }
// }

//carga los numeros a mostrar en secuencia en un vector
//del 1 al 9; 10 es cero; 11 es punto; 0, 12, 13, 14, 15 apagar
void VectorToDisplayStr (char * s_number)
{
    unsigned char len;

    len = strlen(s_number);    //sin el \0
    if (len < LAST_NUMBER)
    {
        UpdateDisplayResetSM();
        strcpy(display_vector_numbers, s_number);
        UpdateDisplayStartSM();
    }
}



//--- end of file ---//
