//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SEND_SEGMENTS.C ################################
//------------------------------------------------------

#include "send_segments.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "spi.h"


// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Module Functions ------------------------------------------------------------
void SendSegment (unsigned char segment)
{
    OE_OFF;
    if (segment & 0x80)
    	SPI_Send_Single (0x01);
    else
    	SPI_Send_Single (0x00);

    segment <<= 1;
    SPI_Send_Single (segment);
    OE_ON;
}


//--- end of file ---//
