//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### OUTPUTS.C ######################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "outputs.h"
#include "hard.h"
#include "stm32f0xx.h"


// Externals variables ---------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Functions ------------------------------------------------------------
void Fplus_On (void)
{
    FPLUS_ON;
}


void Fplus_Off (void)
{
    FPLUS_OFF;
}


void F12plus_On (void)
{
    F12PLUS_ON;
}


void F12plus_Off (void)
{
    F12PLUS_OFF;
}


void F5plus_On (void)
{
    F5PLUS_ON;
}


void F5plus_Off (void)
{
    F5PLUS_OFF;
}

//--- end of file ---//
