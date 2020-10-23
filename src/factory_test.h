//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### FACTORY_TEST.H #################################
//------------------------------------------------------
#ifndef _FACTORY_TEST_H_
#define _FACTORY_TEST_H_

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
    PK_OUTPUT_TO_ACTIVATE,
    PK_OUTPUT_ACTIVE

} porton_kirno_sm_t;




// Exported Module Functions ---------------------------------------------------
void FuncFactoryTest (void);


#endif    /* _FACTORY_TEST_H_ */

//--- end of file ---//
