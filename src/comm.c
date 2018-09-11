//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### COMM.C #######################################
//------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "comm.h"
#include "hard.h"
#include "usart.h"
#include "flash_program.h"

#include <string.h>
#include <stdio.h>


/* Externals variables ---------------------------------------------------------*/
extern const char s_ok [];

extern unsigned char next_pckt;
extern unsigned char file_done;

#ifdef CONFIGURATION_IN_FLASH
extern unsigned char b1t;
extern unsigned char b1r;
extern unsigned char b2t;
extern unsigned char b2r;
extern unsigned char b3t;
extern unsigned char b3r;
extern unsigned char b4t;
extern unsigned char b4r;
#endif

#ifdef CONFIGURATION_IN_SST
extern parameters_typedef param_struct;
#endif


/* Global variables ------------------------------------------------------------*/
const char s_tiempos [] = {"Tiempos\r"};
const char s_256 [] = {"Code Dump\r"};
const char s_512 [] = {"Code Dump2\r"};

#ifdef FUNC_MEM
const char s_read_sst[] = {"Leer SST\r"};
const char s_grabar_sst0[] = {"Grabar SST0\r"};
const char s_grabar_sst1[] = {"Grabar SST1\r"};
const char s_grabar_sst2[] = {"Grabar SST2\r"};
const char s_grabar_sst3[] = {"Grabar SST3\r"};
const char s_grabar_sst4[] = {"Grabar SST4\r"};
const char s_grabar_sst5[] = {"Grabar SST5\r"};
const char s_grabar_sst6[] = {"Grabar SST6\r"};
const char s_grabar_sst7[] = {"Grabar SST7\r"};
const char s_grabar_sst8[] = {"Grabar SST8\r"};
const char s_grabar_sst9[] = {"Grabar SST9\r"};
const char s_grabar_prox[] = {"Proximo\r"};
const char s_grabar_term[] = {"Terminado\r"};
#endif

const char s_grabar [] = {"Grabar,"};


/* Module Functions ------------------------------------------------------------*/
//TODO copiar payload solo en las cuestiones de LCD y no en los comandos
unsigned char InterpretarMsg (unsigned char lstate, char * pStr)	
{
    char str [64];

    if (strncmp(pStr, (const char *) "Starting Goto 115200\r",
                sizeof((const char *) "Starting Goto 115200\r") - 1) == 0)
    {
        return MAIN_TO_MONITORING;
    }

    if (strncmp(pStr, (const char *) "Leaving Goto 9600\r",
                sizeof((const char *) "Leaving Goto 9600\r") - 1) == 0)
    {
        return MAIN_TO_MONITORING_LEAVE;
    }

    if (strncmp(pStr, s_256, sizeof(s_256) - 1) == 0)
    {
        return MAIN_MEMORY_DUMP;
    }

    if (strncmp(pStr, s_512, sizeof(s_512) - 1) == 0)
    {
        return MAIN_MEMORY_DUMP2;
    }

#ifdef CONFIGURATION_IN_FLASH
    if (strncmp(pStr, s_grabar, sizeof(s_grabar) - 1) == 0)
    {
        //
        b1t = ToInt3(pStr + 7);
        b1r = ToInt3(pStr + 11);
        b2t = ToInt3(pStr + 15);
        b2r = ToInt3(pStr + 19);
        b3t = ToInt3(pStr + 23);
        b3r = ToInt3(pStr + 27);
        b4t = ToInt3(pStr + 31);
        b4r = ToInt3(pStr + 35);

        //pruebo con los T
        //sprintf(str, "%d %d %d %d\r\n", b1t,b2t,b3t,b4t);
        //Usart1Send(str);
        //pruebo con los R
        //sprintf(str, "%d %d %d %d\r\n", b1r,b2r,b3r,b4r);
        //Usart1Send(str);
        return MAIN_TO_SAVE_TIME;
    }

    //FUNCIONES PARA GRABAR MEMORIA
    //SST0
    if (strncmp(pStr, s_grabar_sst0, sizeof(s_grabar_sst0) - 1) == 0)
        return MAIN_TO_WRITE_SST0;
    //SST1
    if (strncmp(pStr, s_grabar_sst1, sizeof(s_grabar_sst1) - 1) == 0)
        return MAIN_TO_WRITE_SST1;
    //SST2
    if (strncmp(pStr, s_grabar_sst2, sizeof(s_grabar_sst2) - 1) == 0)
        return MAIN_TO_WRITE_SST2;
    //SST3
    if (strncmp(pStr, s_grabar_sst3, sizeof(s_grabar_sst3) - 1) == 0)
        return MAIN_TO_WRITE_SST3;
    //SST4
    if (strncmp(pStr, s_grabar_sst4, sizeof(s_grabar_sst4) - 1) == 0)
        return MAIN_TO_WRITE_SST4;
    //SST5
    if (strncmp(pStr, s_grabar_sst5, sizeof(s_grabar_sst5) - 1) == 0)
        return MAIN_TO_WRITE_SST5;
    //SST6
    if (strncmp(pStr, s_grabar_sst6, sizeof(s_grabar_sst6) - 1) == 0)
        return MAIN_TO_WRITE_SST6;
    //SST7
    if (strncmp(pStr, s_grabar_sst7, sizeof(s_grabar_sst7) - 1) == 0)
        return MAIN_TO_WRITE_SST7;
    //SST87
    if (strncmp(pStr, s_grabar_sst8, sizeof(s_grabar_sst8) - 1) == 0)
        return MAIN_TO_WRITE_SST8;
    //SST9
    if (strncmp(pStr, s_grabar_sst9, sizeof(s_grabar_sst9) - 1) == 0)
        return MAIN_TO_WRITE_SST9;

    if (strncmp(pStr, s_grabar_prox, sizeof(s_grabar_prox) - 1) == 0)
    {
        //si llega proximo vuelvo al MAIN donde estaba
        next_pckt = 1;
        return lstate;
    }

    if (strncmp(pStr, s_grabar_term, sizeof(s_grabar_term) - 1) == 0)
    {

        Usart1Send((char *) s_ok);
        file_done = 1;
        return lstate;
        //return MAIN_TO_MONITORINGC;
    }

#endif

    if (strncmp(pStr, s_tiempos, sizeof(s_tiempos) - 1) == 0)
    {
        //sprintf(str, "B1T: %d\r\n", p_params->b1t);
#ifdef INFO_IN_FLASH
        sprintf(str, "Tiempos %d,%d,%d,%d,%d,%d,%d,%d,\r\n", b1t, b1r, b2t, b2r, b3t, b3r, b4t, b4r);
#endif
#ifdef INFO_IN_SST
        sprintf(str, "Tiempos %d,%d,%d,%d,%d,%d,%d,%d,\r\n", param_struct.b1t, param_struct.b1r,
                param_struct.b2t, param_struct.b2r,
                param_struct.b3t, param_struct.b3r,
                param_struct.b4t, param_struct.b4r);
#endif
        Usart1Send(str);
    }

    return lstate;
}

unsigned short ToInt3 (char * p)
{
    unsigned short result = 0;

    //leo 3 caracteres y lo paso a unsigned char
    if (((*p >= '0') && (*p <= '9')) &&
        ((*(p + 1) >= '0') && (*(p + 1) <= '9')) &&
        ((*(p + 2) >= '0') && (*(p + 2) <= '9')))
    {
        result = 100 * (*p - '0') + 10 * (*(p + 1) - '0') + (*(p + 2) - '0');
    }
    return result;
}

//--- end of file ---//
