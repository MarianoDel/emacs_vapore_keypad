//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### KEYPAD.H #################################
//------------------------------------------------------
#ifndef _KEYPAD_H_
#define _KEYPAD_H_

// Additional Includes for Configuration ------------------------------


// Defines for Configuration ----------------------------------------
#define KEYPAD_TIMEOUT    50


// Exported Macros and Defines ----------------------------------------
#define NO_KEY 0
#define NINE_KEY 9
#define STAR_KEY 10
#define ZERO_KEY 11
#define POUND_KEY 12

//ESTADOS DEL KEYPAD
#define KNONE				0
#define KRECEIVING_A		1
#define KRECEIVING_B		2
#define KRECEIVING_C		3
#define KRECEIVING_D		4
#define KRECEIVING_E		5
#define KRECEIVING_F		6

#define KNUMBER_FINISH		8
#define KCANCEL				9
#define KTIMEOUT			10

// //estados de la SM del DISPLAY
// typedef enum {
//     DISPLAY_SM_INIT = 0,
//     DISPLAY_SM_SENDING,
//     DISPLAY_SM_SHOWING,
//     DISPLAY_SM_WAITING

// } display_sm_t;



// Exported Module Functions ------------------------------------------
unsigned char UpdateSwitches (void);
unsigned char CheckKeypad (unsigned char *, unsigned char *, unsigned char *, unsigned short *);



#endif    /* _KEYPAD_H_ */

//--- end of file ---//
