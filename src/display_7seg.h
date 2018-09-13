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

// Additional Includes for Configuration ------------------------------


// Defines for Configuration ----------------------------------------
#define DISPLAY_TIMER_IN_ON		800
#define DISPLAY_TIMER_IN_OFF		200


// Exported Macros and Defines ----------------------------------------
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
//cargar nuevos simbolos en ShowNumbers
//y en UpdateDisplaySM si se necesita enviarlos vectorizados

//estados de la SM del DISPLAY
typedef enum {
    DISPLAY_SM_INIT = 0,
    DISPLAY_SM_SENDING,
    DISPLAY_SM_SHOWING,
    DISPLAY_SM_WAITING

} display_sm_t;



// Exported Module Functions ------------------------------------------
void ConvertPositionToDisplay (unsigned short);
void UpdateDisplaySM (void);
void UpdateDisplayResetSM (void);
void UpdateDisplayStartSM (void);
void VectorToDisplayStr (char *);
void ShowNumbers (unsigned char);
void SendSegment (unsigned char);
void ShowNumbersAgain (void);
unsigned char DisplayIsFree (void);


#endif    /* _DISPLAY_7SEG_H_ */

//--- end of file ---//

