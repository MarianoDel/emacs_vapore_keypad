//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### COMM.C #######################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "comm.h"
#include "hard.h"
#include "usart.h"
#include "parameters.h"

#include <string.h>
#include <stdio.h>

#include "gestion.h"


// Externals variables ---------------------------------------------------------
extern unsigned char next_pckt;
extern unsigned char file_done;

extern parameters_typedef param_struct;


// Globals ---------------------------------------------------------------------
#ifdef PROGRAMA_DE_GESTION
const char s_grabar_sst0[] = {"Grabar SST0"};
const char s_grabar_sst1[] = {"Grabar SST1"};
const char s_grabar_sst2[] = {"Grabar SST2"};
const char s_grabar_sst3[] = {"Grabar SST3"};
const char s_grabar_sst4[] = {"Grabar SST4"};
const char s_grabar_sst5[] = {"Grabar SST5"};
const char s_grabar_sst6[] = {"Grabar SST6"};
const char s_grabar_sst7[] = {"Grabar SST7"};
const char s_grabar_sst8[] = {"Grabar SST8"};
const char s_grabar_sst9[] = {"Grabar SST9"};
const char s_grabar_sstb1[] = {"Grabar SSTB1"};
const char s_grabar_sstb3[] = {"Grabar SSTB3"};

const char s_grabar_sstconf[] = {"Grabar Conf SST"};
const char s_grabar_prox[] = {"Proximo"};
const char s_grabar_term[] = {"Terminado"};
const char s_borrar_sst[] = {"Borrar SST"};
// const char s_read_sst[] = {"Leer SST"};
const char s_tiempos [] = {"Tiempos"};
#endif



// Module Functions ------------------------------------------------------------
//TODO copiar payload solo en las cuestiones de LCD y no en los comandos
unsigned char InterpretarMsg (unsigned char lstate, char * pStr)	
{
#ifdef PROGRAMA_DE_GESTION

    if (strncmp(pStr, (const char *) "Starting Goto 115200",
                sizeof((const char *) "Starting Goto 115200") - 1) == 0)
    {
        return GESTION_SM_TO_MONITORING;
    }

    if (strncmp(pStr, (const char *) "Leaving Goto 9600",
                sizeof((const char *) "Leaving Goto 9600") - 1) == 0)
    {
        return GESTION_SM_TO_MONITORING_LEAVE;
    }

    //FUNCIONES PARA GRABAR MEMORIA
    if (strncmp(pStr, s_borrar_sst, sizeof(s_borrar_sst) - 1) == 0)
        return GESTION_SM_TO_FLUSH_SST;

    if (strncmp(pStr, s_grabar_sstconf, sizeof(s_grabar_sstconf) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST_CONF;

    //SST0
    if (strncmp(pStr, s_grabar_sst0, sizeof(s_grabar_sst0) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST0;
    //SST1
    if (strncmp(pStr, s_grabar_sst1, sizeof(s_grabar_sst1) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST1;
    //SST2
    if (strncmp(pStr, s_grabar_sst2, sizeof(s_grabar_sst2) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST2;
    //SST3
    if (strncmp(pStr, s_grabar_sst3, sizeof(s_grabar_sst3) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST3;
    //SST4
    if (strncmp(pStr, s_grabar_sst4, sizeof(s_grabar_sst4) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST4;
    //SST5
    if (strncmp(pStr, s_grabar_sst5, sizeof(s_grabar_sst5) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST5;
    //SST6
    if (strncmp(pStr, s_grabar_sst6, sizeof(s_grabar_sst6) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST6;
    //SST7
    if (strncmp(pStr, s_grabar_sst7, sizeof(s_grabar_sst7) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST7;
    //SST87
    if (strncmp(pStr, s_grabar_sst8, sizeof(s_grabar_sst8) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST8;
    //SST9
    if (strncmp(pStr, s_grabar_sst9, sizeof(s_grabar_sst9) - 1) == 0)
        return GESTION_SM_TO_WRITE_SST9;
    //SSTB1
    if (strncmp(pStr, s_grabar_sstb1, sizeof(s_grabar_sstb1) - 1) == 0)
        return GESTION_SM_TO_WRITE_SSTB1;
    //SSTB3
    if (strncmp(pStr, s_grabar_sstb3, sizeof(s_grabar_sstb3) - 1) == 0)
        return GESTION_SM_TO_WRITE_SSTB3;


    if (strncmp(pStr, s_grabar_prox, sizeof(s_grabar_prox) - 1) == 0)
    {
        //si llega proximo vuelvo al MAIN donde estaba
        next_pckt = 1;
        return lstate;
    }

    if (strncmp(pStr, s_grabar_term, sizeof(s_grabar_term) - 1) == 0)
    {
        Usart1Send((char *) "OK\r\n");
        file_done = 1;
        return lstate;
    }

    if (strncmp(pStr, s_tiempos, sizeof(s_tiempos) - 1) == 0)
    {
        char s_to_send [96] = { 0 };

        sprintf(s_to_send, "Tiempos %d,%d,%d,%d,%d,%d,%d,%d,",
                param_struct.b1t,
                param_struct.b1r,
                param_struct.b2t,
                param_struct.b2r,
                param_struct.b3t,
                param_struct.b3r,
                param_struct.b4t,
                param_struct.b4r);

          if (param_struct.audio_buttons & B1_AUDIO_MASK)
              strcat(s_to_send, "1,");
          else
              strcat(s_to_send, "0,");

          if (param_struct.audio_buttons & B3_AUDIO_MASK)
              strcat(s_to_send, "1,\r\n");
          else
              strcat(s_to_send,"0,\r\n");
        
        Usart1Send(s_to_send);
    }


#endif    // PROGRAMA_DE_GESTION

    // Give posibility to activate alarm from serial port (Activation by SMS)
    if (strncmp(pStr, "ACT_12V ACTIVO", sizeof("ACT_12V ACTIVO") - 1) == 0)
    {
        SetSMS();
        Usart1Send("sms activation!\n");
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


unsigned char sms_activation = 0;
unsigned char CheckSMS (void)
{
    return sms_activation;
}


void ResetSMS (void)
{
    sms_activation = 0;
}


void SetSMS (void)
{
    sms_activation = 1;
}



//--- end of file ---//
