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
#include "usart.h"
// #include "tim.h"
#include "display_7seg.h"
#include "codes.h"
#include "sst25codes.h"
#include "keypad.h"

#include <stdio.h>
#include <string.h>


// Private Types Constants and Macros ------------------------------------------
typedef enum
{
    TEST_INIT,
    TEST_CHECK_BUZZER,
    TEST_CHECK_RF,
    TEST_CHECK_HBRIDGE,
    TEST_CHECK_USART,
    TEST_CHECK_USART_DELAY,    
    TEST_CHECK_MEMORY_WRITE,
    TEST_CHECK_MEMORY_READ,
    TEST_CHECK_KEYPAD,
    TEST_ERROR
        
} TestState_e;


// PA7
#define H_L    ((GPIOA->ODR & 0x0080) != 0)
#define H_L_ON    (GPIOA->BSRR = 0x00000080)
#define H_L_OFF    (GPIOA->BSRR = 0x00800000)

// PA8
#define H_H    ((GPIOA->ODR & 0x0100) != 0)
#define H_H_ON    (GPIOA->BSRR = 0x00000100)
#define H_H_OFF    (GPIOA->BSRR = 0x01000000)

// Externals -------------------------------------------------------------------
extern volatile unsigned short timer_standby;
extern volatile unsigned short siren_timeout;
extern volatile unsigned char usart1_have_data;
extern volatile unsigned char rx1buff[];
extern volatile unsigned char * prx1;


// Globals ---------------------------------------------------------------------


// Private Module Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void FuncFactoryTest (void)    
{
    unsigned char switches = 0;
    unsigned char last_switches = 0;
    unsigned char usart_error_cnt = 0;
    TestState_e test_state = TEST_INIT;
    Display_ResetSM();

    // for test on rf_pin
    unsigned char rf_pin_last = 0;
    unsigned short rf_pin_changes = 0;

    // for tests on hbridge
    unsigned int temp = 0;
    temp = GPIOA->MODER;    //2 bits por pin
    temp &= 0xFFFC3FFF;    //PA7 PA8 push pull
    temp |= 0x00014000;    //
    GPIOA->MODER = temp;
    H_L_OFF;
    H_H_OFF;

    while (1)
    {
        switch (test_state)
        {
        case TEST_INIT:
            Display_ShowNumbers(6);
            BuzzerCommands(BUZZER_SHORT_CMD, 2);
            timer_standby = 900;
            test_state++;
            break;

        case TEST_CHECK_BUZZER:
            if ((!timer_standby) &&
                (BuzzerIsFree())) //espero que termine de enviar el buzzer
            {
                test_state++;
                Display_ShowNumbers(5);
                timer_standby = 1000;
            }
            break;

        case TEST_CHECK_RF:
            if (!timer_standby)
            {
                if (rf_pin_changes > 40)
                {
                    test_state++;
                    Display_ShowNumbers(4);
                }
                else
                    test_state = TEST_ERROR;
            }

            if (rf_pin_last != RX_CODE)
            {
                rf_pin_last = RX_CODE;
                rf_pin_changes++;
            }
            break;

        case TEST_CHECK_HBRIDGE:
            if (!timer_standby)
            {
                timer_standby = 500;
                if (H_L)
                {
                    H_L_OFF;
                    H_H_ON;
                }
                else
                {
                    H_L_ON;
                    H_H_OFF;
                }
            }
            
            if (UpdateSwitches() != NO_KEY)
            {
                H_L_OFF;
                H_H_OFF;
                test_state++;
                Display_ShowNumbers(3);
            }
            break;
            
        case TEST_CHECK_USART:
            if (!timer_standby)
            {
                // send test string
                Usart1Send("Test string for usart1\r\n");
                timer_standby = 200;
                usart_error_cnt ++;
            }

            if (usart1_have_data)            
            {
                usart1_have_data  = 0;
                prx1 = rx1buff;
                if (!strncmp("Test string for usart1", (const char *) rx1buff, 22))
                {
                    test_state++;
                    timer_standby = 900;
                }
            }
            else if (usart_error_cnt > 4)
                test_state = TEST_ERROR;
            
            break;

        case TEST_CHECK_USART_DELAY:
            if (!timer_standby)
            {
                test_state++;
                Display_ShowNumbers(2);
                timer_standby = 900;
            }
            break;
            
        case TEST_CHECK_MEMORY_WRITE:
            if (timer_standby)
                break;
            
            if (SST_WriteCodeToMemory(100, 0x5555) == PASSED)
            {
                timer_standby = 900;
                test_state++;
                Display_ShowNumbers(1);                
            }
            else
                test_state = TEST_ERROR;

            break;

        case TEST_CHECK_MEMORY_READ:
            if (timer_standby)
                break;

            if (SST_CheckIndexInMemory(100) == 0x5555)
            {
                Display_ShowNumbers(0);
                timer_standby = 900;
                test_state++;
            }
            else
                test_state = TEST_ERROR;

            break;

        case TEST_CHECK_KEYPAD:
            if (timer_standby)
                break;
            
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
