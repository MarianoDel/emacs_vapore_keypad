//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### DISPLAY_7SEG.H #################################
//------------------------------------------------------
#ifndef _DISPLAY_7SEG_H_
#define _DISPLAY_7SEG_H_


// Defines for Configuration ----------------------------------------
#define DISPLAY_TIMER_IN_ON		800
#define DISPLAY_TIMER_IN_OFF		200


// Module Exported Types Constants and Macros ----------------------------------
#define DISPLAY_NONE	0xF0
#define DISPLAY_ZERO	0
#define DISPLAY_POINT	10
#define DISPLAY_LINE	11
#define DISPLAY_REMOTE	12
#define DISPLAY_ERROR	13
#define DISPLAY_ERROR2	14

#define DISPLAY_SQR_UP		DISPLAY_REMOTE
#define DISPLAY_SQR_DOWN		15
#define DISPLAY_H        16
#define DISPLAY_S        17
#define DISPLAY_PROG     18

#define SIZEOF_VDISPLAY    DISPLAY_PROG + 1




// Exported Module Functions ------------------------------------------
void Display_ConvertPosition (unsigned short);
void Display_UpdateSM (void);
void Display_ResetSM (void);
void Display_StartSM (void);
void Display_VectorToStr (char *);
void Display_ShowNumbers (unsigned char);
void Display_ShowNumbersAgain (void);
unsigned char Display_IsFree (void);


#endif    /* _DISPLAY_7SEG_H_ */

//--- end of file ---//

