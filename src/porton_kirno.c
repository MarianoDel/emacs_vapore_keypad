//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PORTON_KIRNO.C #################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "porton_kirno.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "usart.h"
#include "tim.h"
#include "display_7seg.h"
#include "codes.h"

#include <stdio.h>

#define OCODE_ON    LED_ON
#define OCODE_OFF    LED_OFF

// Externals -------------------------------------------------------------------
extern volatile unsigned short timer_standby;
extern volatile unsigned short siren_timeout;


// Globals ---------------------------------------------------------------------
porton_kirno_sm_t porton_state = PK_INIT;
unsigned char minutes = 0;


// Private Module Functions ----------------------------------------------------
resp_t PortonKirnoCodes (porton_kirno_codes_t *);

// Module Functions ------------------------------------------------------------
void FuncPortonKirno (void)
{
    porton_kirno_codes_t my_codes;
    char s_send [100] = { 0 };
    resp_t resp = resp_continue;
    

    Usart1Send((char *) "Programa de Porton Kirno - Hard Vapore Keypad\r\n");

    //reset a la SM del display
    Display_ResetSM();
    //apago el display
    Display_ShowNumbers(DISPLAY_PROG);
    // Display_ShowNumbers(DISPLAY_NONE);
    
    BuzzerCommands(BUZZER_LONG_CMD, 2);

    TIM_16_Init();

    
    while (1)
    {
        switch (porton_state)
        {
        case PK_INIT:
            if (!timer_standby)
                porton_state = PK_STAND_BY;

            break;

        case PK_STAND_BY:
            resp = PortonKirnoCodes(&my_codes);
            
            if (resp == resp_ok)
            {
                sprintf(s_send, "finded! rxcode: 0x%x rxlambda: %d rxbits: %d\n",
                        my_codes.code,
                        my_codes.lambda,
                        my_codes.bits);
                Usart1Send(s_send);

#ifdef USE_KIRNO_CODES                
                //chequeo parametros del codigo y activo
                if ((my_codes.bits == 28) &&
                    (my_codes.lambda > 300) &&
                    (my_codes.lambda < 330))
                {
                    // control 1 B3 y B4
                    // control 2 B3 y B4
                    if ((my_codes.code == 0x63d1f45) ||
                        (my_codes.code == 0x63d1f15) ||
                        (my_codes.code == 0x7401045) ||
                        (my_codes.code == 0x7401015))
                    {
                        porton_state = PK_OUTPUT_TO_ACTIVATE;                        
                    }
                }
#endif
#ifdef USE_MEMB_CODES
                if ((my_codes.bits == 12) &&
                    (my_codes.lambda > 440) &&
                    (my_codes.lambda < 500))
                {
                    // controls B1
                    if (my_codes.code == 0x1)
                    {
                        porton_state = PK_OUTPUT_TO_DELAY_ACTIVATE;
                        timer_standby = 10000;    // wait 10 secs
                        Usart1Send("Delay output for 10 secs\n");
                        siren_timeout = 0;
                    }
                }                
#endif
            }
            break;

        case PK_OUTPUT_TO_DELAY_ACTIVATE:
            if (!timer_standby)
            {
                porton_state = PK_OUTPUT_TO_ACTIVATE;                
            }

            if (!siren_timeout)
            {
                BuzzerCommands(BUZZER_SHORT_CMD, 1);
                siren_timeout = 1000;
            }
            break;

        case PK_OUTPUT_TO_ACTIVATE:
            FPLUS_ON;
            minutes = 5;
            siren_timeout = 0;
            Usart1Send("Output is ON for 5 minutes\n");
            porton_state = PK_OUTPUT_ACTIVE;
            break;
            
        case PK_OUTPUT_ACTIVE:
            if (!timer_standby)
            {
                if (minutes)
                {
                    minutes--;
                    timer_standby = 60000;
                }
                else
                {
                    FPLUS_OFF;
                    BuzzerCommands(BUZZER_LONG_CMD, 3);
                    timer_standby = 3000;
                    Usart1Send("Output is OFF\n");
                    porton_state = PK_INIT;
                }
            }

            if ((FPLUS) && (!siren_timeout))
            {
                siren_timeout = 5000;
                BuzzerCommands(BUZZER_HALF_CMD, 2);
            }
            break;
            

        default:
            porton_state = PK_INIT;
            break;

        }
        UpdateBuzzer();
    }
}

typedef enum {
    KC_WAIT_SILENCE_INIT,
    KC_WAIT_SILENCE,
    KC_RX,
    KC_GET_CODE_HT,
    KC_GET_CODE_PT_EV
    
} kirno_codes_t;

kirno_codes_t kc_state = KC_WAIT_SILENCE_INIT;

unsigned char i = 0;
resp_t PortonKirnoCodes (porton_kirno_codes_t * new_code_st)
{
    unsigned int new_code = 0;
    unsigned short new_lambda = 0;
    char s_buf [100] = { 0 };
    resp_t resp = resp_continue;

    
    switch (kc_state)
    {
    case KC_WAIT_SILENCE_INIT:
        CodesWaitFiveReset();
        kc_state++;
        break;
            
    case KC_WAIT_SILENCE:

        resp = CodesWaitFive();

        if (resp == resp_ok)
        {
            //estuve 5ms sin nada
            LED_ON;
            OCODE_ON;

            kc_state = KC_RX;
            // Usart1Send((char *) "nuevo header\n");
            CodesRecvCode16Reset();
            resp = resp_continue;
        }

        if (resp == resp_error)
            kc_state = KC_WAIT_SILENCE_INIT;
            
        break;

    case KC_RX:
        resp = CodesRecvCode16(&i);

        if (resp != resp_continue)
        {
            if (resp == resp_ok)
            {
                // sprintf(s_buf, "bits: %d OK\n", i);
                // Usart1Send(s_buf);
                kc_state = KC_GET_CODE_HT;
                resp = resp_continue;
            }

            if (resp == resp_error)
            {
                // for debug, shows how many bits gets
                // sprintf(s_buf, "bits: %d ERR\n", i);
                // Usart1Send(s_buf);
                kc_state = KC_WAIT_SILENCE_INIT;
            }
                
            LED_OFF;
            OCODE_OFF;
        }
        break;

    case KC_GET_CODE_HT:
        resp = CodesUpdateTransitionsHT(i, &new_code, &new_lambda);
                        
        if (resp == resp_ok)
        {
            sprintf(s_buf, "code: 0x%x lambda: %d bits: %d HT\n",
                    new_code,
                    new_lambda,
                    i);

            Usart1Send(s_buf);
            new_code_st->bits = i;
            new_code_st->code = new_code;
            new_code_st->lambda = new_lambda;

            kc_state = KC_WAIT_SILENCE_INIT;
        }
        else
        {
            kc_state = KC_GET_CODE_PT_EV;
        }
        break;
            
    case KC_GET_CODE_PT_EV:
        resp = CodesUpdateTransitionsPT_EV(i, &new_code, &new_lambda);

        if (resp == resp_ok)
        {
            sprintf(s_buf, "code: 0x%x lambda: %d bits: %d PT or EV\n",
                    new_code,
                    new_lambda,
                    i);

            new_code_st->bits = i;
            new_code_st->code = new_code;
            new_code_st->lambda = new_lambda;

            kc_state = KC_WAIT_SILENCE_INIT;
        }
        else
        {
            sprintf(s_buf, "code error\n");
        }

        Usart1Send(s_buf);
        kc_state = KC_WAIT_SILENCE_INIT;
        break;
            
    default:
        kc_state = KC_WAIT_SILENCE_INIT;
        break;

    }

    return resp;
}

//--- end of file ---//
