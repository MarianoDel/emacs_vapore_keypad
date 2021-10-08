//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS.C ###############################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "func_alarm.h"
#include "parameters.h"

//helper modules
#include "tests_ok.h"
#include "tests_mock_usart.h"

#include <stdio.h>
#include <string.h>

// Module for test Private Types -----------------------------------------------
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
extern func_alarm_state_e alarm_st;
extern char usart1_msg_sended [];
extern unsigned char alarm_button_timer_secs;

// Globals ---------------------------------------------------------------------
parameters_typedef param_struct;
unsigned char audio_state = 0;
int fplus_out = 0;
int f5plus_out = 0;
int f12plus_out = 0;



// Module Auxialiary or Mocked Functions ---------------------------------------
void SirenCommands (unsigned char cmd);
void VectorToSpeak (unsigned char cmd);
void PositionToSpeak (unsigned short posi);
void Fplus_On (void);
void Fplus_Off (void);
void F5plus_On (void);
void F5plus_Off (void);
void F12plus_On (void);
void F12plus_Off (void);


// Module Functions for testing ------------------------------------------------
void Test_Func_Alarm (void);
    

// Module Functions ------------------------------------------------------------


int main(int argc, char *argv[])
{

    Test_Func_Alarm ();
    
}


void Test_Func_Alarm (void)
{
    resp_t resp = resp_continue;


    // Test sms activation
    resp = Func_Alarm_SM (SMS_ALARM, 0, 0);

    if ((resp == resp_continue) &&
        (alarm_st == F_ALARM_BUTTON1) &&
        (strcmp(usart1_msg_sended, "SMS Activo: 911 B1\r\n") == 0))
    {
        printf("Test SMS activation: ");
        PrintOK();
    }
    else
    {
        printf("Test SMS activation: ");
        PrintERR();
    }

    
    printf("Test Reset function: ");
    Func_Alarm_Reset_SM ();
    if (alarm_st == F_ALARM_START)
        PrintOK();
    else
        PrintERR();


    // Test Error on control activation - no button
    resp = Func_Alarm_SM (CONTROL_ALARM, 0, 0);

    if ((resp == resp_error) &&
        (alarm_st == F_ALARM_START))
    {
        printf("Test error on activation no button: ");
        PrintOK();
    }
    else
    {
        printf("Test error on activation no button: ");
        PrintERR();
    }

    // Test Error on control activation - bad position
    resp = Func_Alarm_SM (CONTROL_ALARM, 2000, 1);

    if ((resp == resp_error) &&
        (alarm_st == F_ALARM_START))
    {
        printf("Test error on activation posi error: ");
        PrintOK();
    }
    else
    {
        printf("Test error on activation posi error: ");
        PrintERR();
    }

    // Test Error on control activation - new code in first call
    resp = Func_Alarm_SM (NEWCODE_ALARM, 0, 1);

    if ((resp == resp_error) &&
        (alarm_st == F_ALARM_START))
    {
        printf("Test error on activation newcode first call error: ");
        PrintOK();
    }
    else
    {
        printf("Test error on activation newcode first call error: ");
        PrintERR();
    }


    param_struct.b1r = 5;
    param_struct.b1t = 5;
    param_struct.b2r = 5;
    param_struct.b2t = 5;
    param_struct.b3r = 5;
    param_struct.b3t = 5;
    param_struct.b4r = 5;
    param_struct.b4t = 5;
    // Test Control activation
    int error = 0;
    // for (int i = 0; i < 1001; i++)
    for (int i = 0; i < 2; i++)        
    {
        resp = Func_Alarm_SM (CONTROL_ALARM, i, 1);
        //TODO: puedo tambien comparar el string de activacion enviado
        if ((resp != resp_continue) &&
            (alarm_st != F_ALARM_BUTTON1))
        {
            error = 1;
            printf("Button 1 activation error on: %d", i);
        }
        Func_Alarm_Reset_SM();
    }

    printf("Test CONTROL_ALARM B1 activation: ");    
    if (!error)
        PrintOK();
    else
        PrintERR();

    int loop_number = 0;
    // Test Control activation B1 to B4, output timeout
    for (int button = 1; button <= 4; button++)
    {
        error = 0;
        loop_number = 0;
        Func_Alarm_Reset_SM();    
        for (int i = 0; i < 100; i++)
        {
            resp = Func_Alarm_SM (CONTROL_ALARM, 1, button);

            if (resp == resp_error)
            {
                error = 1;
                printf("B%d activation error on: %d", button, i);
            }
            else if (resp == resp_ok)
            {
                loop_number = i;
                break;
            }
            else
            {
                // empty the timeouts
                if (alarm_button_timer_secs)
                    alarm_button_timer_secs--;
            
            }
        }

        printf("Test CONTROL_ALARM B%d activation: ", button);    
        // if ((!error) &&
        //     (resp = resp_ok) &&
        //     (alarm_st == F_ALARM_START))
        if ((!error) &&
            (resp = resp_ok) &&
            (fplus_out == 0) &&
            (f5plus_out == 0) &&
            (f12plus_out == 0))
        {
            PrintOK();
            printf("B%d on loop number: %d\n", button, loop_number);
        }
        else
            PrintERR();
    }

    // Test Control activation B1 to B4, output B1 to B4 deact
    for (int button = 1; button <= 4; button++)
    {
        error = 0;
        loop_number = 0;
        Func_Alarm_Reset_SM();    
        for (int i = 0; i < 200; i++)
        {
            if (i == 15)    //deact on 15
                resp = Func_Alarm_SM (NEWCODE_ALARM, 1, button);
            else
                resp = Func_Alarm_SM (CONTROL_ALARM, 1, button);

            if (resp == resp_error)
            {
                error = 1;
                printf("B%d activation error on: %d", button, i);
            }
            else if (resp == resp_ok)
            {
                loop_number = i;
                break;
            }
            else
            {
                // empty the timeouts on hundred of miliseconds
                if (alarm_button_timer_secs)
                    alarm_button_timer_secs--;
            }
        }

        printf("Test CONTROL_ALARM B%d activation/deactivation: ", button);    
        // if ((!error) &&
        //     (resp = resp_ok) &&
        //     (alarm_st == F_ALARM_START))
        if ((!error) &&
            (resp = resp_ok) &&
            (fplus_out == 0) &&
            (f5plus_out == 0) &&
            (f12plus_out == 0))
        {
            PrintOK();
            printf("B%d on loop number: %d\n", button, loop_number);
        }
        else
            PrintERR();
    }
    
}


// Auxiliary Mocked Functions --------------------------------------------------
void SirenCommands (unsigned char cmd)
{
    printf("New siren command: %d\n", cmd);
}


void VectorToSpeak (unsigned char cmd)
{
    printf("New vector to speak command: %d\n", cmd);
}


void PositionToSpeak (unsigned short posi)
{
    printf("New position to speak: %d\n", posi);
}


void Fplus_On (void)
{
    printf("fplus on\n");
    fplus_out = 1;
}


void Fplus_Off (void)
{
    printf("fplus off\n");
    fplus_out = 0;
}


void F5plus_On (void)
{
    printf("f5plus on\n");
    f5plus_out = 1;
}


void F5plus_Off (void)
{
    printf("f5plus off\n");
    f5plus_out = 0;
}


void F12plus_On (void)
{
    printf("f12plus on\n");
    f12plus_out = 1;
}


void F12plus_Off (void)
{
    printf("f12plus off\n");
    f12plus_out = 0;
}


//--- end of file ---//


