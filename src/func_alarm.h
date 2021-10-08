//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### FUNC_ALARM.H ###################################
//------------------------------------------------------
#ifndef _FUNC_ALARM_H_
#define _FUNC_ALARM_H_

// Additional Includes for Configuration ---------------------------------------
#include "codes.h"

// Defines for Configuration ---------------------------------------------------


// Exported Types Constants and Macros -----------------------------------------
typedef enum {
    CONTROL_ALARM = 0,
    SMS_ALARM,
    NEWCODE_ALARM

} func_alarm_status_e;




// Module Exported Functions ---------------------------------------------------
resp_t Func_Alarm_SM (func_alarm_status_e status, unsigned short position, unsigned char button);
void Func_Alarm_Reset_SM (void);


#endif    /* _FUNC_ALARM_H_ */

//--- end of file ---//
