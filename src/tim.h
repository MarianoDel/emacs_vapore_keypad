//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### TIM.H ################################
//---------------------------------------------
#ifndef _TIM_H_
#define _TIM_H_


// Module Exported Types Constants and Macros ----------------------------------
#define TIM3_ARR    666
#define TIM1_ARR    666

#define FREQ_1000HZ    1000
#define FREQ_800HZ    1250

#define DUTY_50_1000    500
#define DUTY_50_800    625


// Module Exported Functions ---------------------------------------------------
void TIM14_IC_CH1_OFF (void);
void TIM14_IC_CH1_ON (void);
void TIM14_IC_CNT (unsigned short new_counter);
void Power_Ampli_Ena (void);
void Power_Ampli_Disa (void);
void ChangeAmpli(unsigned short, unsigned short);
void TIM_1_Init (void);
void TIM3_IRQHandler (void);
void TIM_3_Init(void);
void TIM14_IRQHandler (void);
void TIM_14_Init(void);
void TIM16_IRQHandler (void);
void TIM_16_Init(void);
void TIM17_IRQHandler (void);
void TIM_17_Init(void);
void Update_TIM3_CH1 (unsigned short);
void Update_TIM3_CH2 (unsigned short);
void Update_TIM1_CH1 (unsigned short);

void Wait_ms (unsigned short wait);

void Ampli_to_Audio (void);
void Ampli_to_Sirena (void);
void LoadSine(short *);

#define CodeInterruptDisa() TIM14_IC_CH1_OFF()
#define CodeInterruptEna() TIM14_IC_CH1_ON()

#endif    /* _TIM_H_ */

//--- end of file ---//

