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
int enable_audio_outputs = 1;
int enable_gpio_outputs = 1;



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

void Test_Func_Activation_Timeout (int position,
                                   unsigned char button_active);

void Test_Func_Activation_Deact (int position,
                                 unsigned char button_active,
                                 int deact_loop);

void Test_Func_Activation_Override (int position,
                                    unsigned char button_active,
                                    unsigned char button_override,
                                    int override_loop);

void Test_Audio_Enable (int ena);
void Test_Gpio_Enable (int ena);

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

    printf("Test bad alarm state case: ");
    alarm_st = 200;
    resp = Func_Alarm_SM (SMS_ALARM, 0, 0);
    
    if ((resp == resp_continue) &&
        (alarm_st == F_ALARM_START))
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
    param_struct.audio_buttons = 0x05;    //audio button 3 and 1

    // Test Control activation
    Usart1OuputEnable (0);
    Test_Audio_Enable (0);
    Test_Gpio_Enable (0);
    for (int i = 0; i < 1000; i++)
    {
        Test_Func_Activation_Timeout (i, 1);
    }

    Usart1OuputEnable (1);
    // Test Control activation B1 to B4, output timeout
    for (int i = 1; i <= 4; i++)
        Test_Func_Activation_Timeout (1, i);

    // Test Control activation B1 to B3, output B1 to B3 deact
    // B4 can't be deactivate
    Test_Func_Activation_Deact (1, 1, 15);
    Test_Func_Activation_Deact (1, 2, 4);
    Test_Func_Activation_Deact (1, 3, 15);

    // Test control activation B2 override by B1
    Test_Func_Activation_Override (1, 2, 1, 4);
    
    // Test control activation B2 override by B3
    Test_Func_Activation_Override (1, 2, 3, 4);    

    // Test control activation B2 override by SMS
    Test_Func_Activation_Override (1, 2, 5, 4);

    // Test control activation B3 override by B1
    Test_Func_Activation_Override (1, 3, 1, 15);
    
    // Test control activation B3 override by SMS
    Test_Func_Activation_Override (1, 3, 5, 15);    
    
    
    
}

// Test function activation buttons, output timeout
void Test_Func_Activation_Timeout (int position,
                                   unsigned char button_active)
{
    resp_t resp = resp_continue;
    int error = 0;
    int loop_number = 0;

    Func_Alarm_Reset_SM();    

    for (int i = 0; i < 200; i++)
    {
        resp = Func_Alarm_SM (CONTROL_ALARM, position, button_active);

        if (resp == resp_error)
        {
            error = 1;
            printf("B%d activation error on: %d", button_active, i);
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

    printf("Test CONTROL_ALARM %03d B%d activation output by timeout: ",
           position,
           button_active);
    
    char output_msg [100];
    sprintf(output_msg, "Timeout B%d %03d\r\n", button_active, position);

    if ((!error) &&
        (resp = resp_ok) &&
        (fplus_out == 0) &&
        (f5plus_out == 0) &&
        (f12plus_out == 0) &&
        (strcmp(usart1_msg_sended, output_msg) == 0))
    {
        PrintOK();
        printf("B%d on loop number: %d\n", button_active, loop_number);
    }
    else
        PrintERR();
    
}



// Test function activation and deactivation same button
void Test_Func_Activation_Deact (int position,
                                 unsigned char button_active,
                                 int deact_loop)
{
    resp_t resp = resp_continue;
    int error = 0;
    int loop_number = 0;
    int milisenconds = 0;
    Func_Alarm_Reset_SM();    

    for (int i = 0; i < 200; i++)    //hasta 20 segundos
    {
        if (i == deact_loop)    //deact on 15
            resp = Func_Alarm_SM (NEWCODE_ALARM, position, button_active);
        else
            resp = Func_Alarm_SM (CONTROL_ALARM, position, button_active);

        if (resp == resp_error)
        {
            error = 1;
            printf("B%d activation error on: %d", button_active, i);
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

    printf("Test CONTROL_ALARM B%d activation/deactivation: ", button_active);    
    char output_msg [100];
    sprintf(output_msg, "Desactivo: %03d B%d\r\n", position, button_active);

    if ((!error) &&
        (resp = resp_ok) &&
        (fplus_out == 0) &&
        (f5plus_out == 0) &&
        (f12plus_out == 0) &&
        (strcmp(usart1_msg_sended, output_msg) == 0))
    {
        PrintOK();
        printf("B%d on loop number: %d\n", button_active, loop_number);
    }
    else
        PrintERR();

}

// Test function activation by one button override by other
void Test_Func_Activation_Override (int position,
                                    unsigned char button_active,
                                    unsigned char button_override,
                                    int override_loop)
{
    resp_t resp = resp_continue;
    int error = 0;
    int loop_number = 0;
    Func_Alarm_Reset_SM();    
    for (int i = 0; i < 200; i++)
    {
        if (i == override_loop)    //react on override_loop
        {
            if (button_override == 5)
            {
                position = 911;
                button_override = 1;
                resp = Func_Alarm_SM (SMS_ALARM, position, button_override);
            }
            else
                resp = Func_Alarm_SM (NEWCODE_ALARM, position, button_override);
        }
        else
            resp = Func_Alarm_SM (CONTROL_ALARM, position, button_active);

        if (resp == resp_error)
        {
            error = 1;
            printf("B%d activation error on: %d", button_active, i);
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

    printf("Test CONTROL_ALARM B%d activation override B%d: ", button_active, button_override);
    char output_msg [100];
    sprintf(output_msg, "Timeout B%d %03d\r\n", button_override, position);
    if ((!error) &&
        (resp = resp_ok) &&
        (fplus_out == 0) &&
        (f5plus_out == 0) &&
        (f12plus_out == 0) &&
        (strcmp(usart1_msg_sended, output_msg) == 0))
    {
        PrintOK();
        printf("B%d on loop number: %d\n", button_active, loop_number);
    }
    else
        PrintERR();
    
}


void Test_Audio_Enable (int ena)
{
    enable_audio_outputs = ena;
}


void Test_Gpio_Enable (int ena)
{
    enable_gpio_outputs = ena;
}

// Auxiliary Mocked Functions --------------------------------------------------
void SirenCommands (unsigned char cmd)
{
    if (enable_audio_outputs)
        printf("New siren command: %d\n", cmd);
}


void VectorToSpeak (unsigned char cmd)
{
    if (enable_audio_outputs)
        printf("New vector to speak command: %d\n", cmd);
}


void PositionToSpeak (unsigned short posi)
{
    if (enable_audio_outputs)
        printf("New position to speak: %d\n", posi);
}


void Fplus_On (void)
{
    if (enable_gpio_outputs)
        printf("fplus on\n");
    
    fplus_out = 1;
}


void Fplus_Off (void)
{
    if (enable_gpio_outputs)
        printf("fplus off\n");
    
    fplus_out = 0;
}


void F5plus_On (void)
{
    if (enable_gpio_outputs)
        printf("f5plus on\n");
    
    f5plus_out = 1;
}


void F5plus_Off (void)
{
    if (enable_gpio_outputs)
        printf("f5plus off\n");
    
    f5plus_out = 0;
}


void F12plus_On (void)
{
    if (enable_gpio_outputs)
        printf("f12plus on\n");
    
    f12plus_out = 1;
}


void F12plus_Off (void)
{
    if (enable_gpio_outputs)
        printf("f12plus off\n");
    
    f12plus_out = 0;
}


//--- end of file ---//


