//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### RWS317_HARD_CONF.H #################################
//----------------------------------------------------------
#ifndef _RWS317_HARD_CONF_H_
#define _RWS317_HARD_CONF_H_

#include "tim.h"


// Exported Types Constants and Macros -----------------------------------------
#define RxCode_TIM_Disable()	TIM14_IC_CH1_OFF ()
#define RxCode_TIM_Enable() 	TIM14_IC_CH1_ON ()
#define RxCode_TIM_CNT(X)    TIM14_IC_CNT(X)


// Exported Module Functions ---------------------------------------------------

#endif    /* _RWS317_HARD_CONF_H_ */

//--- end of file ---//

