//-----------------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_DISPLAY.C ###############################
//-----------------------------------------------------

// Includes Modules for tests --------------------------------------------------
#include "display_7seg.h"

#include <stdio.h>
#include <string.h>
// #include <math.h>

// auxiliary modules
#include "tests_ok.h"

// Types Constants and Macros --------------------------------------------------


// Externals -------------------------------------------------------------------
volatile unsigned short display_timeout = 0;
extern char display_vector_numbers [];
extern const unsigned char v_display_numbers [];


// Globals ---------------------------------------------------------------------
unsigned char v_show = 0;
unsigned char new_segment = 0;


// Module Functions to Test ----------------------------------------------------
void TEST_Display_7Seg (void);

// Modules Mocked Functions ----------------------------------------------------
void SendSegment (unsigned char);


// Module Functions ------------------------------------------------------------
int main (int argc, char *argv[])
{
    printf("Simple module tests\n");
    TEST_Display_7Seg ();
    

    printf("Module tests ended!\n");

    return 0;
}




// void Display_UpdateSM (void);
// void Display_ShowNumbersAgain (void);


// Tests Functions -------------------------------------------------------------
void TEST_Display_7Seg (void)
{
    unsigned char errors = 0;

    printf("Testing display_7seg module: ");

    //////////////////////////////////////////////////////////
    // Display_ResetSM() Display_StartSM() Display_IsFree() //
    //////////////////////////////////////////////////////////
    Display_ResetSM();
    if (!Display_IsFree())
    {
        errors = 1;
        PrintERR();
        printf("expected 1 = free\n");
    }

    Display_StartSM();
    if (Display_IsFree())
    {
        errors = 1;
        PrintERR();
        printf("expected 0 = working\n");
    }

    ///////////////////////////
    // Display_VectorToStr() //
    ///////////////////////////
    char s_number [] = {"010."};
    Display_VectorToStr (s_number);
    if (strcmp(display_vector_numbers, s_number) != 0)
    {
        errors = 1;
        PrintERR();
        printf("expected == 0 getted != 0\n");        
    }

    char s_number2 [] = {"666666"};
    Display_VectorToStr (s_number2);
    if (strcmp(display_vector_numbers, s_number) != 0)
    {
        errors = 1;
        PrintERR();
        printf("expected == 0 getted != 0\n");        
    }

    ///////////////////////////////
    // Display_ConvertPosition() //
    ///////////////////////////////
    Display_ConvertPosition (10);
    if (strcmp(display_vector_numbers, "010.") != 0)
    {
        errors = 1;
        PrintERR();
        printf("expected 010.\n");        
    }

    Display_ConvertPosition (0);
    if (strcmp(display_vector_numbers, "000.") != 0)
    {
        errors = 1;
        PrintERR();
        printf("expected 000.\n");        
    }

    Display_ConvertPosition (999);
    if (strcmp(display_vector_numbers, "999.") != 0)
    {
        errors = 1;
        PrintERR();
        printf("expected 998.\n");        
    }

    Display_ConvertPosition (2000);
    if (strcmp(display_vector_numbers, "999.") != 0)
    {
        errors = 1;
        PrintERR();
        printf("expected 999. from previous test\n");        
    }
    
    ///////////////////////////
    // Display_ShowNumbers() //
    ///////////////////////////
    for (unsigned char i = 0; i < SIZEOF_VDISPLAY; i++)
    {
        Display_ShowNumbers(i);
        unsigned char disp = v_display_numbers[i];

        if (v_show != disp)
        {
            errors = 1;
            PrintERR();
            printf("expected 0x%02x getted 0x%02x\n", disp, v_show);
            break;
        }
    }

    
    ////////////////////////////////
    // Display_ShowNumbersAgain() //
    ////////////////////////////////
    unsigned char disp = v_display_numbers[SIZEOF_VDISPLAY - 1];
    Display_ShowNumbersAgain();
    if (v_show != disp)
    {
        errors = 1;
        PrintERR();
        printf("expected 0x%02x getted 0x%02x\n", disp, v_show);
    }


    ////////////////////////
    // Display_UpdateSM() //
    ////////////////////////
    Display_ResetSM();
    Display_ConvertPosition (111);

    if (Display_IsFree())
    {
        errors = 1;
        PrintERR();
        printf("expected 0 = working\n");
    }

    unsigned char vect_show [10] = { 0 };
    unsigned char show_cntr = 0;
    new_segment = 0;
    while (!Display_IsFree())
    {
        Display_UpdateSM();
        display_timeout = 0;
        if (new_segment)
        {
            new_segment = 0;
            vect_show[show_cntr] = v_show;
            show_cntr++;
        }
    }

    if (show_cntr != 8)
    {
        errors = 1;
        PrintERR();
        printf("expected 8 getted %d\n", show_cntr);
    }
    // printf("ended with index: %d\nvalues are: ", show_cntr);    
    // for (unsigned char i = 0; i < show_cntr; i++)
    // {
    //     printf("0x%02x ", vect_show[i]);
    // }

    
    
    if (!errors)
        PrintOK();
}


// Mocked Functions ------------------------------------------------------------
void SendSegment (unsigned char a)
{
    v_show = a;
    new_segment = 1;
    // printf("0x%02x ", a);
}



//--- end of file ---//


