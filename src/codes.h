//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### CODES.H ################################
//---------------------------------------------

#ifndef CODES_H_
#define CODES_H_

#include "gpio.h"
#include "hard.h"
#include "stm32f0xx.h"

//--- Hardware Configs ------------------------------
#define CODES_TIMER_CNT    (TIM16->CNT)
#define DATA_PIN_ACTIVE_WITH_ONE
// #define DATA_PIN_ACTIVE_WITH_ZERO




#ifdef DATA_PIN_ACTIVE_WITH_ONE
#define DATA_PIN          (RX_CODE)
#else
#define DATA_PIN          (!RX_CODE)
#endif


// #define RX_CODE_PLLUP_ON Gpio5PullUpOn()
// #define RX_CODE_PLLUP_OFF Gpio5PullUpOff()

#define SIZEOF_BUFF_TRANS	64


typedef enum {
    resp_continue = 0,
    resp_ok,
    resp_error
    
} resp_t;

//Wait States
typedef enum {
	CW_INIT = 0,
	CW_WAITING_IN_ZERO,
        CW_WAITING_IN_ONE

} wait_state_t;

//Code States
typedef enum {
	C_INIT = 0,
	C_SEND_PILOT_A,
	C_SEND_PILOT_B,
	C_SENDING,
	C_SEND_ONE_A,
	C_SEND_ONE_B,
	C_SEND_ONE_C,
	C_SEND_ZERO_A,
	C_SEND_ZERO_B,
	C_SEND_ZERO_C,
	C_FINISH

} CodeStateTX_t;

typedef enum {
	C_RXINIT = 0,
	C_RXINIT_PULLUP,
	C_RXWAIT_PILOT_A,
	C_RXWAIT_PILOT_B,
	C_RXWAIT_BITS_B,
	C_RXWAIT_BITS_C,
	C_RXERROR,
	C_RXOK

} CodeStateRX_t;

//--- Funciones del Modulo ---
unsigned char SendCode16 (unsigned int, unsigned char, unsigned short);
unsigned char SendCode16FixLambda (unsigned int, unsigned char, unsigned short, unsigned short);
unsigned char SendCode16WithPilot (unsigned int, unsigned char, unsigned short, unsigned short);
void SendCode16Reset (void);
resp_t CodesRecvCode16 (unsigned char *);
resp_t CodesRecvCode16Ones (unsigned char *);
void CodesRecvCode16Reset (void);
resp_t CodesUpdateTransitionsHT (unsigned char, unsigned int *, unsigned short *);
resp_t CodesUpdateTransitionsPT_EV (unsigned char, unsigned int *, unsigned short *);
resp_t CodesWaitFive (void);
resp_t CodesWaitFive_Up_or_Down (unsigned char *);
void CodesWaitFiveReset (void);

#endif /* CODES_H_ */
