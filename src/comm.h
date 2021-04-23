//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### COMM.H #######################################
//------------------------------------------------------
#ifndef _COMM_H_
#define _COMM_H_

// Exported Macros and Defines ----------------------------------------


// Exported Module Functions ------------------------------------------
unsigned char InterpretarMsg (unsigned char, char *);
unsigned short ToInt3 (char *);
unsigned char CheckSMS (void);
void ResetSMS (void);
void SetSMS (void);

#endif    /* _COMM_H_ */

//--- end of file ---//

