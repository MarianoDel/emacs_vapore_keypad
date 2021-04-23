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
#include "tests_ok.h"

#include <stdio.h>
// #include <stdlib.h>
#include <string.h>


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned char usart1_have_data = 0;
char new_uart_msg [200] = { 0 };
char last_uart_sended [200] = { 0 };



// Module Auxialiary Functions -------------------------------------------------
void Usart1Send (char * msg);
unsigned char Usart1ReadBuffer (unsigned char * bout, unsigned short max_len);


// Module Functions for testing ------------------------------------------------
void Test_Comms (void);
    

// Module Functions ------------------------------------------------------------


int main(int argc, char *argv[])
{

    Test_Comms ();
    
}


void Test_Comms (void)
{
    // Test ok acknowledge
    CommFlushAck();
    usart1_have_data = 1;
    strcpy(new_uart_msg, "ok\n");

    UpdateCommunications();
    
    if ((CommCheckOK()) &&
        (!CommCheckNOK()) &&
        (usart1_have_data == 0))
    {
        PrintOK();
    }
    else
        PrintERR();

    // Test nok acknowledge
    CommFlushAck();
    usart1_have_data = 1;
    strcpy(new_uart_msg, "nok\n");

    UpdateCommunications();
    
    if ((CommCheckNOK()) &&
        (!CommCheckOK()) &&
        (usart1_have_data == 0))
    {
        PrintOK();
    }
    else
        PrintERR();

    // Test others strings
    CommFlushAck();
    usart1_have_data = 1;
    strcpy(new_uart_msg, "fido\n");

    UpdateCommunications();
    
    if ((CommCheckNOK()) &&
        (!CommCheckOK()) &&
        (usart1_have_data == 0))
    {
        PrintOK();
    }
    else
        PrintERR();
    
}


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


