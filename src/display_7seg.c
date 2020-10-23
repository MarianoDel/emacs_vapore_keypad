//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DISPLAY_7SEG.C #################################
//------------------------------------------------------

#include "display_7seg.h"
#include "send_segments.h"

#include <string.h>
#include <stdio.h>


// Module Private Types Constants and Macros -----------------------------------
#define LAST_NUMBER    6    //size of display numbers buffer 3 digits + point + '\0'


//	dp g f e d c b a
//bits   7 6 5 4 3 2 1 0
//negados
const unsigned char v_display_numbers [] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
                                             0x80, 0x98, 0x7F, 0xBF, 0x9C, 0x06, 0xB0, 0xA3,
                                             0x8B, 0x92, 0x8C };

// DISPLAY State Machine
typedef enum {
    DISPLAY_SM_INIT = 0,
    DISPLAY_SM_SENDING,
    DISPLAY_SM_SHOWING,
    DISPLAY_SM_WAITING

} display_sm_t;

// Externals -------------------------------------------------------------------
// for the timer
extern volatile unsigned short display_timeout;


// Globals ---------------------------------------------------------------------
unsigned char display_last_digit = 0;
char display_vector_numbers [LAST_NUMBER];
char * p_vector_numbers;

display_sm_t display_state = DISPLAY_SM_INIT;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void Display_ShowNumbersAgain (void)
{
    Display_ShowNumbers (display_last_digit);
}


//beware read the switches disables de displayed number
//system calls ShowNumbersAgain to solve this
void Display_ShowNumbers (unsigned char number)	//from 0 to 9 are numbers; 10 to 15 are other symbols
{
    display_last_digit = number;
    if ((number != DISPLAY_NONE) && (number < SIZEOF_VDISPLAY))
        number = v_display_numbers[number];
    else
        number = 0xFF;
    
    SendSegment (number);
}


unsigned char Display_IsFree (void)
{
    if (display_state == DISPLAY_SM_INIT)
        return 1;
    else
        return 0;
}


void Display_ResetSM (void)
{
    p_vector_numbers = display_vector_numbers;
    display_state = DISPLAY_SM_INIT;
}


void Display_StartSM (void)
{
    display_state = DISPLAY_SM_SENDING;
}


//show secuences of numbers by calling to ShowNumbers
//the numbers must be setted by calling to VectorToDisplay()
//ONLY Numbers and the POINT
void Display_UpdateSM (void)
{

    switch (display_state)
    {
    case DISPLAY_SM_INIT:
        break;

    case DISPLAY_SM_SENDING:
        if (*p_vector_numbers == '.')
            Display_ShowNumbers(DISPLAY_POINT);
        else if (*p_vector_numbers == 'h')
            Display_ShowNumbers(DISPLAY_H);
        else if (*p_vector_numbers == 's')
            Display_ShowNumbers(DISPLAY_S);
        else
            Display_ShowNumbers(*p_vector_numbers - '0');
        
        p_vector_numbers++;
        display_state++;
        display_timeout = DISPLAY_TIMER_IN_ON;
        break;

    case DISPLAY_SM_SHOWING:
        if (!display_timeout)
        {
            Display_ShowNumbers(DISPLAY_NONE);
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
//acepta numeros en formato char y el punto
//en general 3 digitos hasta LAST_NUMBER
void Display_VectorToStr (char * s_number)
{
    unsigned char len;

    len = strlen(s_number);    //sin el \0
    if (len < LAST_NUMBER)
    {
        Display_ResetSM();
        strcpy(display_vector_numbers, s_number);
        Display_StartSM();
    }
}


void Display_ConvertPosition (unsigned short pos)
{
    char buff [6] = { '\0' };
    
    if (pos > 999)
        return;

    sprintf(buff, "%03d.", pos);
    Display_VectorToStr(buff);
    
}


//--- end of file ---//
