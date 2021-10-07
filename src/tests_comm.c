//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS.C ###############################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "comm.h"
#include "hard.h"    //for kind of program

// Tests helper modules
#include "tests_ok.h"


#include <stdio.h>
// #include <stdlib.h>
#include <string.h>


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned char usart1_have_data = 0;
char new_uart_msg [200] = { 0 };
char last_uart_sended [200] = { 0 };

#ifdef PROGRAMA_DE_GESTION
#include "gestion.h"
#include "parameters.h"

unsigned char next_pckt = 0;
unsigned char file_done = 0;
parameters_typedef param_struct;

#endif


// Module Auxialiary Functions -------------------------------------------------
void Usart1Send (char * msg);
unsigned char Usart1ReadBuffer (unsigned char * bout, unsigned short max_len);


// Module Functions for testing ------------------------------------------------
void Test_Comms_Normal (void);
void Test_Comms_Gestion (void);
    

// Module Functions ------------------------------------------------------------


int main(int argc, char *argv[])
{
    Test_Comms_Normal ();

#ifdef PROGRAMA_DE_GESTION
    Test_Comms_Gestion ();
#endif    
}


void Test_Comms_Normal (void)
{
    printf("Testing Comms en Programa Normal o Bucle\n");

    unsigned char lstate = 0;

    printf("Check SMS set: ");
    SetSMS();
    if (CheckSMS() == 1)
        PrintOK();
    else
        PrintERR();

    printf("Check SMS reset: ");
    ResetSMS();
    if (CheckSMS() == 0)
        PrintOK();
    else
        PrintERR();
    
    printf("Check string error: ");
    ResetSMS();
    lstate = InterpretarMsg (lstate, "ACT_12V");

    if ((lstate == 0) && (CheckSMS() == 0))
        PrintOK();
    else
        PrintERR();

    printf("Check string ok: ");
    ResetSMS();
    lstate = InterpretarMsg (lstate, "ACT_12V ACTIVO");

    if ((lstate == 0) && (CheckSMS() == 1))
        PrintOK();
    else
        PrintERR();

    printf("Check conversion to int3: ");
    char number_str [10] = { 0 };
    int number = 0;
    int error = 0;
    for (int i = 0; i < 1000; i++)
    {
        sprintf(number_str, "%03d", i);
        number = ToInt3 (number_str);

        if (number != i)
        {
            printf("number error on %d getted %d\n", i, number);
            error = 1;
        }
    }
    
    if (!error)
        PrintOK();
    else
        PrintERR();
    
}

#ifdef PROGRAMA_DE_GESTION
void Test_Comms_Gestion (void)
{
    printf("Testing Comms en Programa de Gestion\n");

    unsigned char lstate = 0;
    
    printf("Check Enter Monitoring: ");
    lstate = InterpretarMsg (lstate, "Starting Goto 115200");
    if (lstate == GESTION_SM_TO_MONITORING)
        PrintOK();
    else
        PrintERR();

    printf("Check Leaving Monitoring: ");
    lstate = InterpretarMsg (lstate, "Leaving Goto 9600");
    if (lstate == GESTION_SM_TO_MONITORING_LEAVE)
        PrintOK();
    else
        PrintERR();

    printf("Check Memory functions\n");
    printf(" Borrar: ");
    lstate = InterpretarMsg (lstate, "Borrar SST");
    if (lstate == GESTION_SM_TO_FLUSH_SST)
        PrintOK();
    else
        PrintERR();

    printf(" Grabar Conf: ");
    lstate = InterpretarMsg (lstate, "Grabar Conf SST");
    if (lstate == GESTION_SM_TO_WRITE_SST_CONF)
        PrintOK();
    else
        PrintERR();

    printf(" Grabar Numeros: ");
    char audios_str [100] = { 0 };
    int errores = 0;
    for (int i = 0; i < 10; i++)
    {
        sprintf (audios_str, "Grabar SST%d", i);
        lstate = InterpretarMsg (lstate, audios_str);
        if (lstate != (GESTION_SM_TO_WRITE_SST0 + i))
        {
            printf("error on %d\n", i);
            errores = 1;
        }
    }
    if (!errores)
        PrintOK();
    else
        PrintERR();

    printf(" Grabar Audio B1: ");
    lstate = InterpretarMsg (lstate, "Grabar SSTB1");
    if (lstate == GESTION_SM_TO_WRITE_SSTB1)
        PrintOK();
    else
        PrintERR();

    printf(" Grabar Audio B3: ");
    lstate = InterpretarMsg (lstate, "Grabar SSTB3");
    if (lstate == GESTION_SM_TO_WRITE_SSTB3)
        PrintOK();
    else
        PrintERR();

    printf("Paquete Proximo: ");
    next_pckt = 0;
    lstate = 0;
    lstate = InterpretarMsg (lstate, "Proximo");
    if ((lstate == 0) && (next_pckt == 1))
        PrintOK();
    else
        PrintERR();

    printf("Paquete Terminado: ");
    file_done = 0;
    lstate = 0;
    lstate = InterpretarMsg (lstate, "Terminado");
    if ((lstate == 0) && (file_done == 1))
        PrintOK();
    else
        PrintERR();

    printf("Paquete de Tiempos: ");
    lstate = 0;
    errores = 0;
    char buffer_to_send [100];
    for (int i = 0; i < 256; i++)
    {
        param_struct.b1t = i;
        param_struct.b1r = i;
        param_struct.b2t = i;
        param_struct.b2r = i;
        param_struct.b3t = i;
        param_struct.b3r = i;
        param_struct.b4t = i;
        param_struct.b4r = i;
        param_struct.audio_buttons = i;

        sprintf(buffer_to_send, "Tiempos %d,%d,%d,%d,%d,%d,%d,%d,",
                param_struct.b1t,
                param_struct.b1r,
                param_struct.b2t,
                param_struct.b2r,
                param_struct.b3t,
                param_struct.b3r,
                param_struct.b4t,
                param_struct.b4r);

        if (param_struct.audio_buttons & B1_AUDIO_MASK)
            strcat(buffer_to_send, "1,");
        else
            strcat(buffer_to_send, "0,");

        if (param_struct.audio_buttons & B3_AUDIO_MASK)
            strcat(buffer_to_send, "1,\r\n");
        else
            strcat(buffer_to_send,"0,\r\n");

        memset(last_uart_sended, 0, sizeof(last_uart_sended));
        lstate = InterpretarMsg (lstate, "Tiempos");
        if ((lstate != 0) ||
            (strcmp(last_uart_sended, buffer_to_send) != 0))
        {
            printf("error on %d getted %s\n", i, last_uart_sended);
            errores = 1;
        }
    }
    if (!errores)
        PrintOK();
    else
        PrintERR();

}
#endif

unsigned char Usart1ReadBuffer (unsigned char * bout, unsigned short max_len)
{
    unsigned char len = 0;
    len = strlen(new_uart_msg);
    if (max_len > len)
        strcpy(bout, new_uart_msg);
    else
        printf("error on Usart1ReadBuffer max_len\n");

    return len;
}


void Usart1Send (char * msg)
{
    strcpy(last_uart_sended, msg);
}



//--- end of file ---//


