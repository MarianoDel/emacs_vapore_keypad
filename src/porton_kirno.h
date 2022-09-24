//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### PORTON_KIRNO.H #################################
//------------------------------------------------------
#ifndef _PORTON_KIRNO_H_
#define _PORTON_KIRNO_H_

// Additional Includes for Configuration ---------------------------------------


// Defines for Configuration ---------------------------------------------------


// Exported Macros and Constants -----------------------------------------------
typedef struct {
    unsigned char bits;
    unsigned int code;
    unsigned short lambda;
    
} porton_kirno_codes_t;

typedef enum {
    PK_INIT = 0,
    PK_STAND_BY,
    PK_OUTPUT_TO_DELAY_ACTIVATE,
    PK_OUTPUT_TO_ACTIVATE,    
    PK_OUTPUT_ACTIVE

} porton_kirno_sm_t;




// Exported Module Functions ---------------------------------------------------
void FuncPortonKirno (void);


#endif    /* _PORTON_KIRNO_H_ */

//--- end of file ---//
