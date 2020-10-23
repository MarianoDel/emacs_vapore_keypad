//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### FACTORY_TEST.C #################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "factory_test.h"
#include "hard.h"
#include "stm32f0xx.h"
// #include "usart.h"
// #include "tim.h"
#include "display_7seg.h"
#include "codes.h"
#include "sst25codes.h"
#include "keypad.h"

#include <stdio.h>


// Private Types Constants and Macros ------------------------------------------
typedef enum
{
    TEST_INIT,
    TEST_CHECK_BUZZER,
    TEST_CHECK_MEMORY_WRITE,
    TEST_CHECK_MEMORY_READ,
    TEST_CHECK_KEYPAD,
    TEST_ERROR
        
} TestState_e;


// Externals -------------------------------------------------------------------
extern volatile unsigned short timer_standby;
extern volatile unsigned short siren_timeout;



// Globals ---------------------------------------------------------------------


// Private Module Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void FuncFactoryTest (void)    
{
    unsigned char switches = 0;
    unsigned char last_switches = 0;
    TestState_e test_state = TEST_INIT;
    Display_ResetSM();


    while (1)
    {
        switch (test_state)
        {
        case TEST_INIT:
            Display_ShowNumbers(3);
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            timer_standby = 900;
            test_state++;
            break;

        case TEST_CHECK_BUZZER:
            if ((!timer_standby) &&
                (BuzzerIsFree())) //espero que termine de enviar el buzzer
            {
                test_state++;
            }
            break;

        case TEST_CHECK_MEMORY_WRITE:
            if (SST_WriteCodeToMemory(100, 0x5555) == PASSED)
            {
                timer_standby = 900;
                test_state++;
            }
            else
                test_state = TEST_ERROR;

            Display_ShowNumbers(2);
//				  test_state++;
            break;

        case TEST_CHECK_MEMORY_READ:
            if (!timer_standby)			//grabo memoria
            {
                if (SST_CheckIndexInMemory(100) == 0x5555)
                {
                    Display_ShowNumbers(0);
                    timer_standby = 900;
                    test_state++;
                }
                else
                    test_state = TEST_ERROR;

                Display_ShowNumbers(1);
            }
            break;

        case TEST_CHECK_KEYPAD:
            if (!timer_standby)
            {
                // switches = ReadSwitches();
                switches = UpdateSwitches();

                if (switches == NO_KEY)
                {
                    Display_ShowNumbers(DISPLAY_NONE);
                    last_switches = switches;
                }
                else
                {
                    if (last_switches != switches)
                    {
                        last_switches = switches;
                        if (switches == ZERO_KEY)
                            Display_ShowNumbers(DISPLAY_ZERO);
                        else if (switches == STAR_KEY)
                            Display_ShowNumbers(DISPLAY_SQR_UP);
                        else if (switches == POUND_KEY)
                            Display_ShowNumbers(DISPLAY_SQR_DOWN);
                        else
                        {
                            Display_ShowNumbers(switches);
                        }
                        BuzzerCommands(BUZZER_SHORT_CMD, 1);
                    }
                }
            }
            break;

        case TEST_ERROR:
            if (!timer_standby)			//me quedo trabado mostrando el error
            {
                BuzzerCommands(BUZZER_SHORT_CMD, 1);
                timer_standby = 700;
            }
            break;


        default:
            test_state = TEST_INIT;
            break;
        }

        UpdateBuzzer();

    }
}

//--- end of file ---//
