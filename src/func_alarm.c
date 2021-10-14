//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### FUNC_ALARM.C ###################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "func_alarm.h"
// #include "hard.h"
// #include "stm32f0xx.h"
#include "usart.h"
// #include "tim.h"
// #include "display_7seg.h"
#include "codes.h"
#include "parameters.h"
#include "rws317.h"

#include "outputs.h"

#include <stdio.h>
#include <string.h>


// Module Private Types Constants and Macros -----------------------------------
typedef enum {
    F_ALARM_START = 0,
    F_ALARM_BUTTON1,
    F_ALARM_BUTTON1_A,
    F_ALARM_BUTTON1_B,
    F_ALARM_BUTTON1_C,
    F_ALARM_BUTTON1_D,
    F_ALARM_BUTTON1_FINISH,
    F_ALARM_BUTTON1_FINISH_B,
    F_ALARM_BUTTON2,
    F_ALARM_BUTTON2_A,
    F_ALARM_BUTTON2_B,
    F_ALARM_BUTTON2_C,
    F_ALARM_BUTTON2_D,
    F_ALARM_BUTTON2_E,
    F_ALARM_BUTTON2_FINISH,

    F_ALARM_BUTTON3,
    F_ALARM_BUTTON3_A,
    F_ALARM_BUTTON3_B,
    F_ALARM_BUTTON3_C,
    F_ALARM_BUTTON3_D,
    F_ALARM_BUTTON3_FINISH,
    F_ALARM_BUTTON3_FINISH_B,

    F_ALARM_BUTTON4,
    F_ALARM_BUTTON4_A,
    F_ALARM_BUTTON4_FINISH
    
} func_alarm_state_e;


// Externals -------------------------------------------------------------------
extern parameters_typedef param_struct;
extern unsigned char audio_state;

extern void SirenCommands(unsigned char);
extern void VectorToSpeak (unsigned char);
extern void PositionToSpeak(unsigned short);
// Globals ---------------------------------------------------------------------
func_alarm_state_e alarm_st = F_ALARM_START;
unsigned short last_position_one_or_three = 0;
unsigned short last_position_two = 0;
unsigned char repetition_cntr = 0;
unsigned char alarm_button_timer_secs = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
resp_t Func_Alarm_SM (func_alarm_status_e status, unsigned short position, unsigned char button)
{
    resp_t resp = resp_continue;
    char str[50];

    switch (alarm_st)
    {
    case F_ALARM_START:
        if (status == SMS_ALARM)
        {
            Usart1Send("SMS Activo: 911 B1\r\n");
            last_position_one_or_three = 911;
            alarm_st = F_ALARM_BUTTON1;
            repetition_cntr = param_struct.b1r;
            alarm_button_timer_secs = ACT_DESACT_IN_SECS;	//2 segundos OK y buena distancia 20-5-15
#ifdef USE_F12_PLUS_WITH_SM
            //modificacion 24-01-2019 F12PLUS espera 10 segundos y se activa 5 segundos
            F12_State_Machine_Start();
#endif
        }
        else if (status == CONTROL_ALARM)
        {
            if ((position >= 0) && (position <= 1023))
            {
                sprintf(str, (char *) "Activo: %03d ", position);
                if (button == 1)
                {
                    last_position_one_or_three = position;
                    alarm_st = F_ALARM_BUTTON1;
                    strcat(str, (char *) "B1\r\n");
                    repetition_cntr = param_struct.b1r;
#ifdef USE_F12_PLUS_WITH_SM
                    //modificacion 24-01-2019 F12PLUS espera 10 segundos y se activa 5 segundos
                    F12_State_Machine_Start();
#endif

                }
                else if (button == 2)
                {
                    //original boton 2
                    last_position_two = position;
                    alarm_st = F_ALARM_BUTTON2;
                    strcat(str, (char *) "B2\r\n");
                }
                else if (button == 3)
                {
                    last_position_one_or_three = position;
                    alarm_st = F_ALARM_BUTTON3;
                    strcat(str, (char *) "B3\r\n");
                    repetition_cntr = param_struct.b3r;
                }
                else if (button == 4)
                {
                    alarm_st = F_ALARM_BUTTON4;
                    strcat(str, (char *) "B4\r\n");
                    repetition_cntr = param_struct.b4r;
                }
                else
                {
                    Usart1Send("CONTROL ALARM with invalid button?\n");
                    return resp_error;                    
                }

                alarm_button_timer_secs = ACT_DESACT_IN_SECS;	//2 segundos OK y buena distancia 20-5-15
                Usart1Send(str);
            }
            else    // CONTROL ALARM with invalid position?
            {
                Usart1Send("CONTROL ALARM with invalid position?\n");
                resp = resp_error;
            }
        }
        else    // NEWCODE_ALARM error?
        {
            Usart1Send("New code alarm on first call!!!\n");
            resp = resp_error;
        }
        break;

    case F_ALARM_BUTTON1:
        Fplus_On();
        F5plus_On();
#ifdef USE_F12_PLUS_ON_BUTTON1
        F12plus_On();
#endif

        SirenCommands(SIREN_MULTIPLE_UP_CMD);
        alarm_st++;
        break;

    case F_ALARM_BUTTON1_A:
        if (!alarm_button_timer_secs)
        {
            alarm_button_timer_secs = param_struct.b1t;
            alarm_st++;
        }
        break;

    case F_ALARM_BUTTON1_B:
        //me quedo esperando que apaguen o timer
#ifdef PROGRAMA_NORMAL
        if ((status == NEWCODE_ALARM) &&
            (button == 1))    // check only for button 1
        {
            sprintf(str, "Desactivo: %03d B1\r\n", position);
            Usart1Send(str);
            alarm_st = F_ALARM_BUTTON1_FINISH;
        }
#endif

#ifdef PROGRAMA_DE_BUCLE
        if ((status == NEWCODE_ALARM) &&
            (button == 4))    // check only for button 4
        {
            sprintf(str, "Desactivo: %03d B4\r\n", position);
            Usart1Send(str);
            alarm_st = F_ALARM_BUTTON1_FINISH;
        }
#endif
        
        if (!alarm_button_timer_secs)
        {
            //tengo timeout, avanzo al audio
            alarm_st++;
        }
        break;

    case F_ALARM_BUTTON1_C:
        //paso el audio y descuento un ciclo
        if (repetition_cntr > 1)
        {
            repetition_cntr--;
            SirenCommands(SIREN_STOP_CMD);

            if (param_struct.audio_buttons & B1_AUDIO_MASK)
                VectorToSpeak('a');
        
            PositionToSpeak(last_position_one_or_three);
            alarm_st++;
        }
        else
        {
            sprintf(str, "Timeout B1 %03d\r\n", last_position_one_or_three);
            Usart1Send(str);
            alarm_st = F_ALARM_BUTTON1_FINISH;
        }
        break;

    case F_ALARM_BUTTON1_D:
        if (audio_state == AUDIO_INIT)
        {
            //termino de enviar audio
            alarm_st = F_ALARM_BUTTON1;
        }
        break;

    case F_ALARM_BUTTON1_FINISH:
        SirenCommands(SIREN_STOP_CMD);
        Fplus_Off();
        F5plus_Off();
#ifdef USE_F12_PLUS_WITH_SM
        F12_State_Machine_Reset();
#else
        F12plus_Off();
#endif

        PositionToSpeak(last_position_one_or_three);
        alarm_st++;
        break;

    case F_ALARM_BUTTON1_FINISH_B:
        if (audio_state == AUDIO_INIT)
        {
            //termino de enviar audio
            resp = resp_ok;
        }
        break;

    case F_ALARM_BUTTON2:		//solo enciendo reflectores
        Fplus_On();

        alarm_st++;
#ifdef HT6P20B2_FOURTH_BUTTON
        alarm_button_timer_secs = 4;	//4 segundos overdrive
#endif
        break;

    case F_ALARM_BUTTON2_A:		//espero los primeros 2 segundos
#ifdef HT6P20B2_FOURTH_BUTTON
        if (!alarm_button_timer_secs)
        {
            alarm_button_timer_secs = 2;
            alarm_st++;
        }
#else
        alarm_st++;
#endif
        break;

    case F_ALARM_BUTTON2_B:
#ifdef HT6P20B2_FOURTH_BUTTON
        if ((status == NEWCODE_ALARM) &&
            (button == 2))    // check only for button 2 to simulate button 4
        {
            if (position == last_position_two)
                alarm_st = F_ALARM_BUTTON2_D;
        }
#endif
        if (!alarm_button_timer_secs)
        {
            alarm_button_timer_secs = param_struct.b2t;
            alarm_st++;
        }
        break;

    case F_ALARM_BUTTON2_C:
        //me quedo esperando que apaguen o timer

#ifdef PROGRAMA_NORMAL
        if (status == SMS_ALARM)
        {
            strcpy(str, "SMS Activo: 911 B1\r\n");
            last_position_one_or_three = 911;
            Usart1Send(str);
            repetition_cntr = param_struct.b1r;
            alarm_button_timer_secs = ACT_DESACT_IN_SECS;
            
            alarm_st = F_ALARM_BUTTON1;
        }
        
        if (status == NEWCODE_ALARM)
        {
            if (button == 2)	//reviso solo boton 2
            {
                sprintf(str, "Desactivo: %03d B2\r\n", position);
                Usart1Send(str);
                alarm_st = F_ALARM_BUTTON2_FINISH;
            }

            //reviso el boton1
            if (button == 1)
            {
                sprintf(str, "Activo: %03d B1\r\n", position);
                last_position_one_or_three = position;
                Usart1Send(str);
                repetition_cntr = param_struct.b1r;
                alarm_button_timer_secs = ACT_DESACT_IN_SECS;

                alarm_st = F_ALARM_BUTTON1;
            }

            if (button == 3)		//reviso el boton
            {
                sprintf(str, "Activo: %03d B3\r\n", position);
                Usart1Send(str);

                last_position_one_or_three = position;
                repetition_cntr = param_struct.b3r;
                alarm_button_timer_secs = ACT_DESACT_IN_SECS;

                alarm_st = F_ALARM_BUTTON3;
            }
        }
#endif
#ifdef PROGRAMA_DE_BUCLE
        if (status == NEWCODE_ALARM)
        {
            if (button == 1)		//reviso el boton
            {
                sprintf(str, "Activo: %03d B1\r\n", position);
                Usart1Send(str);

                last_position_one_or_three = position;
                repetition_cntr = param_struct.b1r;
                alarm_button_timer_secs = ACT_DESACT_IN_SECS;

                alarm_st = F_ALARM_BUTTON1;
            }

            if (button == 3)		//reviso el boton
            {
                sprintf(str, "Activo: %03d B3\r\n", position);
                Usart1Send(str);

                last_position_one_or_three = position;
                repetition_cntr = param_struct.b3r;
                alarm_button_timer_secs = ACT_DESACT_IN_SECS;

                alarm_st = F_ALARM_BUTTON3;
            }

            if (button == 4)	//reviso boton 4 para desactivar
            {
                sprintf(str, "Desactivo: %03d B4\r\n", position);
                Usart1Send(str);
                alarm_st = F_ALARM_BUTTON2_FINISH;
            }
        }
#endif

        if (!alarm_button_timer_secs)
        {
            //tengo timeout, apago reflectores
            sprintf(str, "Timeout B2 %03d\r\n", last_position_two);
            Usart1Send(str);
            alarm_st = F_ALARM_BUTTON2_FINISH;
        }
        break;

    case F_ALARM_BUTTON2_D:    //este caso funciona solo con HT y si el que apaga es el mismo que prende
        SirenCommands(SIREN_STOP_CMD);
        PositionToSpeak(last_position_one_or_three);
        alarm_st++;
        break;

    case F_ALARM_BUTTON2_E:
        if (audio_state == AUDIO_INIT)
        {
            //termino de enviar audio
            alarm_st++;
        }
        break;

    case F_ALARM_BUTTON2_FINISH:
        Fplus_Off();
        resp = resp_ok;
        break;

    case F_ALARM_BUTTON3:
        Fplus_On();
//			F5plus_On();
//			F12plus_On();
        //SirenCommands(SIREN_SINGLE_CMD);
        SirenCommands(SIREN_SINGLE_CHOPP_CMD);
        //alarm_button_timer_secs = 5;	//5 segundos suena seguro EVITA PROBLEMAS EN LA VUELTA
        alarm_st++;
        break;

    case F_ALARM_BUTTON3_A:
        if (!alarm_button_timer_secs)
        {
            alarm_button_timer_secs = param_struct.b3t;
            alarm_st++;
        }
        break;

    case F_ALARM_BUTTON3_B:
        //me quedo esperando que apaguen o timer

#ifdef PROGRAMA_NORMAL
        if (status == SMS_ALARM)
        {
            strcpy(str, "SMS Activo: 911 B1\r\n");
            last_position_one_or_three = 911;
            Usart1Send(str);
            repetition_cntr = param_struct.b1r;
            alarm_button_timer_secs = ACT_DESACT_IN_SECS;
            
            alarm_st = F_ALARM_BUTTON1;
        }
        
        if (status == NEWCODE_ALARM)
        {
            if (button == 3)	//reviso solo boton 3
            {
                sprintf(str, "Desactivo: %03d B3\r\n", position);
                Usart1Send(str);
                alarm_st = F_ALARM_BUTTON3_FINISH;
            }

            //reviso el boton1
            if (button == 1)
            {
                sprintf(str, "Activo: %03d B1\r\n", position);
                last_position_one_or_three = position;
                Usart1Send(str);
                repetition_cntr = param_struct.b1r;
                alarm_button_timer_secs = ACT_DESACT_IN_SECS;

                alarm_st = F_ALARM_BUTTON1;
            }
        }
#endif

#ifdef PROGRAMA_DE_BUCLE
        if (button == 1)		//reviso el boton
        {
            sprintf(str, "Activo: %03d B1\r\n", position);
            Usart1Send(str);

            last_position_one_or_three = position;
            repetition_cntr = param_struct.b1r;
            alarm_button_timer_secs = ACT_DESACT_IN_SECS;

            alarm_st = F_ALARM_BUTTON1;
        }

        if (button == 4)	//reviso solo boton 4
        {
            sprintf(str, "Desactivo: %03d B4\r\n", position);
            Usart1Send(str);
            alarm_st = F_ALARM_BUTTON3_FINISH;
        }
#endif

        if (!alarm_button_timer_secs)
        {
            //tengo timeout, avanzo al audio

            alarm_st++;
        }
        break;

    case F_ALARM_BUTTON3_C:
        // descuento un ciclo y paso el audio
        if (repetition_cntr > 1)
        {
            repetition_cntr--;
            SirenCommands(SIREN_STOP_CMD);

            if (param_struct.audio_buttons & B3_AUDIO_MASK)
                VectorToSpeak('b');
        
            PositionToSpeak(last_position_one_or_three);
            alarm_st++;
        }
        else
        {
            sprintf(str, "Timeout B3 %03d\r\n", last_position_one_or_three);
            Usart1Send(str);
            alarm_st = F_ALARM_BUTTON3_FINISH;
        }
        break;

    case F_ALARM_BUTTON3_D:
        if (audio_state == AUDIO_INIT)
        {
            //termino de enviar audio
            alarm_st = F_ALARM_BUTTON3;
        }
        break;

    case F_ALARM_BUTTON3_FINISH:
        SirenCommands(SIREN_STOP_CMD);
//			F12plus_Off();
        Fplus_Off();

        PositionToSpeak(last_position_one_or_three);
        alarm_st++;
        break;

    case F_ALARM_BUTTON3_FINISH_B:
        if (audio_state == AUDIO_INIT)
        {
            //termino de enviar audio
            resp = resp_ok;
        }
        break;

    case F_ALARM_BUTTON4:
        SirenCommands(SIREN_STOP_CMD);
        PositionToSpeak(last_position_one_or_three);
        alarm_st++;
        break;

    case F_ALARM_BUTTON4_A:
        if (audio_state == AUDIO_INIT)
        {
            //termino de enviar audio
            alarm_st++;
            sprintf(str, "Timeout B4 %03d\r\n", last_position_one_or_three);
            Usart1Send(str);
        }
        break;

    case F_ALARM_BUTTON4_FINISH:
        resp = resp_ok;
        break;

    default:
        alarm_st = 0;
        break;
    }

#ifdef USE_F12_PLUS_WITH_SM
    F12_State_Machine();
#endif

    return resp;
}


void Func_Alarm_Reset_SM (void)
{
    alarm_st = F_ALARM_START;
}


//--- end of file ---//

