//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SST25CODES.H ###################################
//------------------------------------------------------
#ifndef _SST25CODES_H_
#define _SST25CODES_H_

// Additional Includes for Configuration ------------------------------
#include "rws317.h"


// Exported Macros and Defines ----------------------------------------
#define SST_CheckCodeInMemory(X)	SST_CheckBaseMask((X), 0xFFFFFFFF)

#ifdef EV1527_BASE_CODE
#define SST_CheckBaseCodeInMemory(X)	SST_CheckBaseMask((X), EV1527_CODE_MASK)
#define SST_CheckButtonInCode(X)	SST_CheckButton((X), EV1527_BUTTON_MASK)
#endif

#ifdef HT6P20B2_BASE_CODE
#define SST_CheckBaseCodeInMemory(X)	SST_CheckBaseMask((X), HT6P20B2_CODE_MASK)
#define SST_CheckButtonInCode(X)	SST_CheckButton((X), HT6P20B2_BUTTON_MASK)
#endif

#define OFFSET_CODES_256	0x2000
#define OFFSET_CODES_512	0x3000
#define OFFSET_CODES_768	0x4000
#define OFFSET_CODES_1024	0x5000

#define CODES_POSI_256		256
#define CODES_POSI_512		512
#define CODES_POSI_768		768
#define CODES_POSI_1024		1024

// Exported Module Functions ------------------------------------------
unsigned char SST_EraseAllMemory (void);
unsigned int SST_CheckIndexInMemory (unsigned short);
unsigned short SST_CheckBaseMask (unsigned int, unsigned int);
unsigned char SST_WriteCodeToMemory (unsigned short, unsigned int);
void SST_MemoryDump (unsigned int);
unsigned char SST_CheckButton (unsigned int, unsigned int);


#endif /* _SST25CODES_H_ */

//--- end of file ---//

