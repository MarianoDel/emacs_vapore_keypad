//----------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### HARD.C #################################
//----------------------------------------------
#include "hard.h"
#include "stm32f0xx.h"


// Module Private Types & Macros -----------------------------------------------
#define TT_BUZZER_BIP_SHORT		50
#define TT_BUZZER_BIP_SHORT_WAIT	100
#define TT_BUZZER_BIP_HALF		200
#define TT_BUZZER_BIP_HALF_WAIT		500
#define TT_BUZZER_BIP_LONG		1200
#define TT_BUZZER_BIP_LONG_WAIT		1500


// Buzzer Bips States
typedef enum
{    
    BUZZER_WAIT_COMMANDS = 0,
    BUZZER_MARK,
    BUZZER_SPACE,
    BUZZER_TO_STOP
    
} buzzer_state_t;


// Externals -------------------------------------------------------------------
extern volatile unsigned short buzzer_timeout;


// Globals ---------------------------------------------------------------------
//para el buzzer
buzzer_state_t buzzer_state = BUZZER_WAIT_COMMANDS;
unsigned char buzzer_multiple = 0;
unsigned short buzzer_timer_reload_mark = 0;
unsigned short buzzer_timer_reload_space = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void BuzzerCommands(unsigned char command, unsigned char multiple)
{
    if (command == BUZZER_STOP_CMD)
        buzzer_state = BUZZER_TO_STOP;
    else if (command == BUZZER_LONG_CMD)
    {
        buzzer_timer_reload_mark = TT_BUZZER_BIP_LONG;
        buzzer_timer_reload_space = TT_BUZZER_BIP_LONG_WAIT;
    }
    else if (command == BUZZER_HALF_CMD)
    {
        buzzer_timer_reload_mark = TT_BUZZER_BIP_HALF;
        buzzer_timer_reload_space = TT_BUZZER_BIP_HALF_WAIT;
    }
    else
    {
        buzzer_timer_reload_mark = TT_BUZZER_BIP_SHORT;
        buzzer_timer_reload_space = TT_BUZZER_BIP_SHORT_WAIT;
    }

    buzzer_state = BUZZER_MARK;
    buzzer_timeout = 0;
    buzzer_multiple = multiple;
}


void UpdateBuzzer (void)
{
    switch (buzzer_state)
    {
        case BUZZER_WAIT_COMMANDS:
            break;

        case BUZZER_MARK:
            if (!buzzer_timeout)
            {
                BUZZER_ON;
                buzzer_state++;
                buzzer_timeout = buzzer_timer_reload_mark;
            }
            break;

        case BUZZER_SPACE:
            if (!buzzer_timeout)
            {
                if (buzzer_multiple > 1)
                {
                    buzzer_multiple--;
                    BUZZER_OFF;
                    buzzer_state = BUZZER_MARK;
                    buzzer_timeout = buzzer_timer_reload_space;
                }
                else
                    buzzer_state = BUZZER_TO_STOP;
            }
            break;
            
        case BUZZER_TO_STOP:
        default:
            BUZZER_OFF;
            buzzer_state = BUZZER_WAIT_COMMANDS;
            break;
    }
}

//--- end of file ---//
