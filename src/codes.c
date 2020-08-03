//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### CODES.C ################################
//---------------------------------------------

#include "codes.h"
#include "hard.h"
#include "tim.h"
#include "stm32f0xx.h"

// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------
unsigned int bitmask;
unsigned short lambda;

unsigned short bits_t [SIZEOF_BUFF_TRANS];
unsigned char bits_c;

unsigned char last_rx_value = 0;

CodeStateTX_t send_state = 0;
CodeStateRX_t recv_state = 0;
wait_state_t wait_state = CW_INIT;


// Module Functions ------------------------------------------------------------
void CodesWaitFiveReset (void)
{
    wait_state = CW_INIT;
}

resp_t CodesWaitFive (void)
{
    resp_t resp = resp_continue;
    
    switch (wait_state)
    {
    case CW_INIT:
        CODES_TIMER_CNT = 0;
        wait_state++;
        break;

    case CW_WAITING_IN_ZERO:
        if (DATA_PIN)
        {
            wait_state = CW_INIT;
            resp = resp_error;
        }
        
        if (CODES_TIMER_CNT > 4999)
        {
            wait_state = CW_INIT;
            resp = resp_ok;
        }
        break;

    case CW_WAITING_IN_ONE:
        break;
    }
        
    return resp;
}

//espera 5 segundos con la lines baja o alta
//devuelve resp_ok y el valor de la linea en up_down
resp_t CodesWaitFive_Up_or_Down (unsigned char * up_down)
{
    resp_t resp = resp_continue;
    
    switch (wait_state)
    {
    case CW_INIT:
        CODES_TIMER_CNT = 0;

        if (DATA_PIN)
            wait_state = CW_WAITING_IN_ONE;
        else
            wait_state = CW_WAITING_IN_ZERO;

        break;

    case CW_WAITING_IN_ZERO:
        if (DATA_PIN)
        {
            wait_state = CW_INIT;
            resp = resp_error;
        }
                
        if (CODES_TIMER_CNT > 4999)
        {
            wait_state = CW_INIT;
            *up_down = 0;
            resp = resp_ok;
        }
        break;

    case CW_WAITING_IN_ONE:
        if (!DATA_PIN)
        {
            wait_state = CW_INIT;
            resp = resp_error;
        }
                
        if (CODES_TIMER_CNT > 4999)
        {
            wait_state = CW_INIT;
            *up_down = 1;
            resp = resp_ok;
        }
        break;
    }
        
    return resp;
}

//resetea la SM de RecvCode
inline void CodesRecvCode16Reset (void)
{
	// DATA_PIN_PLLUP_OFF;
	// RX_EN_OFF;
	// TIM4Disable();

	recv_state = C_RXINIT;
}

//Recibe puntero a cantidad de bits; por ahora sale ok con 12, 24 o 28 BITS
//contesta resp_continue si falta, resp_ok si termino
//resp_error en error de bits o timeout
resp_t CodesRecvCode16 (unsigned char * bits)
{
    resp_t resp = resp_continue;

    switch (recv_state)
    {
    case C_RXINIT:
        recv_state = C_RXINIT_PULLUP;
        CODES_TIMER_CNT = 0;
        // TIM4Enable();
        bits_c = 0;
        break;

    case C_RXINIT_PULLUP:
        recv_state = C_RXWAIT_PILOT_A;
        
        break;

    case C_RXWAIT_PILOT_A:
        //espera transciciones

        if (DATA_PIN)	//tengo transicion inicio pilot            
        {
            recv_state = C_RXWAIT_PILOT_B;
            CODES_TIMER_CNT = 0;
            bits_c = 0;
        }
        break;

    case C_RXWAIT_PILOT_B:
        if (CODES_TIMER_CNT > 3000)	//pasaron 3mseg sin nada
            recv_state = C_RXERROR;

        if (!DATA_PIN)	//tengo pilot y primera transicion bits            
        {
            bits_t[bits_c] = CODES_TIMER_CNT;
            CODES_TIMER_CNT = 0;
            bits_c++;
            recv_state = C_RXWAIT_BITS_B;	//salto la primera transicion
        }
        break;

    case C_RXWAIT_BITS_B:
        //segunda transcicion de bit
        if (CODES_TIMER_CNT > 3000)	//pasaron 3mseg sin nada, puede ser el final del codigo
        {
            bits_c -= 1;	//ajusto pilot
            bits_c >>= 1;	//2 transciciones 1 bit
            if ((bits_c == 12) ||
                (bits_c == 24) ||
                (bits_c == 28))
                recv_state = C_RXOK;
            else
                recv_state = C_RXERROR;

        }

        if (DATA_PIN)	//tengo segunda transcicion bit            
        {
            bits_t[bits_c] = CODES_TIMER_CNT;
            CODES_TIMER_CNT = 0;

            if (bits_c < SIZEOF_BUFF_TRANS)
            {
                bits_c++;
                recv_state = C_RXWAIT_BITS_C;
            }
            else
                recv_state = C_RXERROR;

        }
        break;

    case C_RXWAIT_BITS_C:
        //tercera transcicion de bit, primera del proximo
        if (CODES_TIMER_CNT > 3000)	//pasaron 3mseg sin nada
            recv_state = C_RXERROR;

        if (!DATA_PIN)	//tengo segunda transcicion bit y primera del proximo            
        {
            bits_t[bits_c] = CODES_TIMER_CNT;
            CODES_TIMER_CNT = 0;

            if (bits_c < SIZEOF_BUFF_TRANS)
            {
                bits_c++;
                recv_state = C_RXWAIT_BITS_B;
            }
            else
                recv_state = C_RXERROR;

        }
        break;

    case C_RXERROR:
        //termine recepcion con error
        resp = resp_error;
        *bits = bits_c;        
        // *bits = 0;
        break;

    case C_RXOK:
        resp = resp_ok;
        *bits = bits_c;
        break;

    default:
        recv_state = C_RXINIT;
        break;
    }
    return resp;
}

resp_t CodesRecvCode16Ones (unsigned char * bits)
{
    resp_t resp = resp_continue;

    switch (recv_state)
    {
    case C_RXINIT:
        recv_state = C_RXINIT_PULLUP;
        CODES_TIMER_CNT = 0;
        // TIM4Enable();
        bits_c = 0;
        break;

    case C_RXINIT_PULLUP:
        recv_state = C_RXWAIT_PILOT_A;
        
        break;

    case C_RXWAIT_PILOT_A:
        //espera transciciones, la primera va a 0

        if (!DATA_PIN)	//tengo transicion inicio pilot            
        {
            recv_state = C_RXWAIT_PILOT_B;
            CODES_TIMER_CNT = 0;
            bits_c = 0;
        }
        break;

    case C_RXWAIT_PILOT_B:
        if (CODES_TIMER_CNT > 3000)	//pasaron 3mseg sin nada
            recv_state = C_RXERROR;

        if (DATA_PIN)	//tengo pilot y primera transicion bits            
        {
            bits_t[bits_c] = CODES_TIMER_CNT;
            CODES_TIMER_CNT = 0;
            bits_c++;
            recv_state = C_RXWAIT_BITS_B;	//salto la primera transicion
        }
        break;

    case C_RXWAIT_BITS_B:
        //segunda transcicion de bit
        if (CODES_TIMER_CNT > 3000)	//pasaron 3mseg sin nada, puede ser el final del codigo
        {
            bits_c -= 1;	//ajusto pilot
            bits_c >>= 1;	//2 transciciones 1 bit
            if ((bits_c == 12) ||
                (bits_c == 24) ||
                (bits_c == 28))
                recv_state = C_RXOK;
            else
                recv_state = C_RXERROR;

        }

        if (!DATA_PIN)	//tengo segunda transcicion bit            
        {
            bits_t[bits_c] = CODES_TIMER_CNT;
            CODES_TIMER_CNT = 0;

            if (bits_c < SIZEOF_BUFF_TRANS)
            {
                bits_c++;
                recv_state = C_RXWAIT_BITS_C;
            }
            else
                recv_state = C_RXERROR;

        }
        break;

    case C_RXWAIT_BITS_C:
        //tercera transcicion de bit, primera del proximo
        if (CODES_TIMER_CNT > 3000)	//pasaron 3mseg sin nada
            recv_state = C_RXERROR;

        if (DATA_PIN)	//tengo segunda transcicion bit y primera del proximo            
        {
            bits_t[bits_c] = CODES_TIMER_CNT;
            CODES_TIMER_CNT = 0;

            if (bits_c < SIZEOF_BUFF_TRANS)
            {
                bits_c++;
                recv_state = C_RXWAIT_BITS_B;
            }
            else
                recv_state = C_RXERROR;

        }
        break;

    case C_RXERROR:
        //termine recepcion con error
        resp = resp_error;
        *bits = bits_c;        
        // *bits = 0;
        break;

    case C_RXOK:
        resp = resp_ok;
        *bits = bits_c;
        break;

    default:
        recv_state = C_RXINIT;
        break;
    }
    return resp;
}

//Recibe cantidad de bits
//contesta con punteros a codigo, lambda rx
//contesta con resp_ok o resp_error cuando valida el codigo
resp_t CodesUpdateTransitionsHT (unsigned char bits, unsigned int * rxcode, unsigned short * lambda)
{
    resp_t resp = resp_ok;
    unsigned char i;
    unsigned char transitions;
    unsigned char tot_lambda;
    unsigned int tot_time = 0;
    unsigned short lambda15;

    *rxcode = 0;
    //tengo 2 transiciones por bit
    transitions = bits * 2;
    for (i = 0; i < transitions; i++)
        tot_time += bits_t[i + 1];		//todas las transciones sin el pilot

    tot_lambda = bits * 3;
    *lambda = tot_time / tot_lambda;
    lambda15 = *lambda * 3;
    lambda15 >>= 1;

    //compenso diferencias en el timer para lambda 30us
    *lambda -= 20;

    //compenso offset de bits
    bits -= 1;
    for (i = 0; i < transitions; i += 2)
    {
        //veo si es 0
        if ((bits_t[i + 1] < lambda15) && (bits_t[i + 2] > lambda15))
        {
            *rxcode &= 0xFFFFFFFE;
            if (bits)
                *rxcode <<= 1;
            bits--;
        }
        //veo si es 1
        else if ((bits_t[i + 1] > lambda15) && (bits_t[i + 2] < lambda15))
        {
            *rxcode |= 1;
            if (bits)
                *rxcode <<= 1;
            bits--;
        }
        //es un error
        else
        {
            i = transitions;
            resp = resp_error;
        }
    }

    return resp;
}

//Recibe cantidad de bits
//contesta con punteros a codigo, lambda rx
//contesta con resp_ok o resp_error cuando valida el codigo
resp_t CodesUpdateTransitionsPT_EV (unsigned char bits, unsigned int * rxcode, unsigned short * lambda)
{
    resp_t resp = resp_ok;
    unsigned char i;
    unsigned char transitions;
    unsigned char tot_lambda;
    unsigned int tot_time = 0;
    unsigned short lambda15;

    *rxcode = 0;
    //tengo 2 transiciones por bit
    transitions = bits * 2;
    for (i = 0; i < transitions; i++)
        tot_time += bits_t[i];		//todas las transiciones, no usa pilot

    tot_lambda = bits * 3;
    *lambda = tot_time / tot_lambda;
    lambda15 = *lambda * 3;
    lambda15 >>= 1;

    //compenso diferencias en el timer para lambda 30us
    // *lambda -= 20;

    //compenso offset de bits
    bits -= 1;
    for (i = 0; i < transitions; i += 2)
    {
        //veo si es 0
        if ((bits_t[i] < lambda15) && (bits_t[i + 1] > lambda15))
        {
            *rxcode &= 0xFFFFFFFE;
            if (bits)
                *rxcode <<= 1;
            bits--;
        }
        //veo si es 1
        else if ((bits_t[i] > lambda15) && (bits_t[i + 1] < lambda15))
        {
            *rxcode |= 1;
            if (bits)
                *rxcode <<= 1;
            bits--;
        }
        //es un error
        else
        {
            i = transitions;
            resp = resp_error;
        }
    }

    return resp;
}


//--- end of file ---//
