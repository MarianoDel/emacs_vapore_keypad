//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### GESTION.C #######################################
//------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "gestion.h"
#include "hard.h"
#include "stm32f0xx.h"
#include "usart.h"
#include "sst25.h"
#include "tim.h"
#include "flash_program.h"
// #include "spi.h"
#include "display_7seg.h"


// Externals -------------------------------------------------------------------
extern volatile unsigned short timer_standby;
extern volatile unsigned char binary_full;
extern mem_bkp_typedef memory_backup;
extern filesystem_typedef files;
extern unsigned char next_pckt;
extern unsigned char file_done;


// Globals ---------------------------------------------------------------------
gestion_sm_t gestion_state = GESTION_SM_INIT;
unsigned int file_size = 0;

//respuestas a los envios desde la PC
const char s_1024 [] = {"1024 bytes\r\n"};
const char s_1024_conf [] = {"1024 config bytes\r\n"};
const char s_ok [] = {"OK\r\n"};
const char s_ok_finish_conf [] = {"FINISH CONF\r\n"};

unsigned int * pfile_position;
unsigned int * pfile_size;
unsigned char file_last_action = 0;
#define LAST_ACTION_SAVE_FILESYSTEM    1
#define LAST_ACTION_SAVE_CONFIGURATION    2

// Private Module Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void FuncGestion (void)
{
    Usart1Send((char *) "STM32F030K6 Memory Manager Program\r\n");

    //cargo los valores de memoria
    if (readJEDEC() == 0)
    {
        while (1)
        {
            Usart1Send((char *) "Memory WRONG!\r\n");
            Wait_ms(2000);
            Usart1Send((char *) "Memory with wrong JEDEC\r\n");
            Wait_ms(2000);
        }
    }

    Usart1Send((char *) "Memory JEDEC is good\r\n");
    Wait_ms(100);

    ShowFileSystem();

#ifdef INFO_IN_SST
    LoadConfiguration();
    ShowConfiguration();
#endif

    Display_ShowNumbers(DISPLAY_PROG);
    
    while (1)
    {
        switch (gestion_state)
        {
        case GESTION_SM_INIT:
            gestion_state = GESTION_SM_MAIN;
            break;

        case GESTION_SM_MAIN:
            break;

        case GESTION_SM_TO_MONITORING:
            Usart1Send((char *) "Monitoring on 115200 confirmed\r\n");
            Display_ShowNumbers(DISPLAY_REMOTE);
            LoadConfiguration();
            Wait_ms(50);

            if (readJEDEC())
                Usart1Send((char *) "Memory OK\r\n");
            else
                Usart1Send((char *) "Memory WRONG!\r\n");
            Wait_ms(50);
            
            if (readNVM(0) != 0xFF)
                Usart1Send((char *) "Filesystem seems OK\r\n");
            else
                Usart1Send((char *) "No Filesystem finded\r\n");
            Wait_ms(50);

            if (readNVM(OFFSET_CONFIGURATION) != 0xFF)
                Usart1Send((char *) "Configuration seems OK\r\n");
            else
                Usart1Send((char *) "No Configuration finded\r\n");
            Wait_ms(50);
            
            gestion_state = GESTION_SM_IN_MONITORING;
            break;

        case GESTION_SM_IN_MONITORING:
            break;

        case GESTION_SM_TO_FLUSH_SST:
            if (readJEDEC())
            {
                Usart1Send((char *) "Memory OK...");
                clearNVM();
                Usart1Send((char *) " - Complete Blank\r\n");
            }
            else
                Usart1Send((char *) "Memory WRONG!\r\n");
            
            gestion_state = GESTION_SM_IN_MONITORING;
            break;

        case GESTION_SM_TO_WRITE_SST_CONF:
            if (readJEDEC())
            {
                Usart1Send((char *) "Memory OK...");
                Clear4KNVM(OFFSET_CONFIGURATION);
                Usart1Send((char *) " - Blank 4K\r\n");
                Wait_ms(50);

                //reuse of files.posi0 for save config
                // files.posi0 = OFFSET_CONFIGURATION;
                // pfile_position = &files.posi0;
                // file_size = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST_CONFA;
            }
            else
            {
                Usart1Send((char *) "Memory WRONG!\r\n");
                gestion_state = GESTION_SM_IN_MONITORING;
            }
            break;

        case GESTION_SM_TO_WRITE_SST_CONFA:
            Usart1Send((char *) s_1024_conf);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST_CONFB:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, OFFSET_CONFIGURATION);
                Usart1Send((char *) s_ok_finish_conf);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST_CONFC:	//inicializo la nueva configuracion
            LoadConfiguration();
            gestion_state = GESTION_SM_IN_MONITORING;
            break;

        case GESTION_SM_TO_WRITE_SST0:
            if (readJEDEC())
            {
                Usart1Send((char *) "Memory OK - clearing all!\r\n");
                clearNVM();

                files.posi0 = OFFSET_FIRST_FILE;
                pfile_position = &files.posi0;
                pfile_size = &files.length0;
                file_size = 0;
                gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            }
            else
            {
                Usart1Send((char *) "Memory WRONG!\r\n");
                gestion_state = GESTION_SM_IN_MONITORING;
            }
            break;

        case GESTION_SM_GET_READY_FOR_CHUNK:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state = GESTION_SM_WAIT_BINARY_CHUNK;
            break;
            
        case GESTION_SM_WAIT_BINARY_CHUNK:    // get the chunk bytes and save it to mem
            if (binary_full)
            {
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, *pfile_position + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//end of chunk and process
                gestion_state = GESTION_SM_WAIT_NEXT_CHUNK;
            }
            break;

        case GESTION_SM_WAIT_NEXT_CHUNK:    // wait next chunk or finish
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            }

            if (file_done)
            {
                file_done = 0;
                *pfile_size = file_size;

                if (file_last_action)
                {
                    file_last_action = 0;
                    Usart1Send((char *) "Filesystem saved\r\n");
                    SaveFilesIndex();
                }
                
                gestion_state = GESTION_SM_IN_MONITORING;
            }
            break;

        case GESTION_SM_TO_WRITE_SST1:            
            files.posi1 = files.posi0 + files.length0;
            pfile_position = &files.posi1;
            pfile_size = &files.length1;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST2:
            files.posi2 = files.posi1 + files.length1;
            pfile_position = &files.posi2;
            pfile_size = &files.length2;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST3:
            files.posi3 = files.posi2 + files.length2;
            pfile_position = &files.posi3;
            pfile_size = &files.length3;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST4:
            files.posi4 = files.posi3 + files.length3;
            pfile_position = &files.posi4;
            pfile_size = &files.length4;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST5:
            files.posi5 = files.posi4 + files.length4;
            pfile_position = &files.posi5;
            pfile_size = &files.length5;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST6:
            files.posi6 = files.posi5 + files.length5;
            pfile_position = &files.posi6;
            pfile_size = &files.length6;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST7:
            files.posi7 = files.posi6 + files.length6;
            pfile_position = &files.posi7;
            pfile_size = &files.length7;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST8:
            files.posi8 = files.posi7 + files.length7;
            pfile_position = &files.posi8;
            pfile_size = &files.length8;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SST9:
            files.posi9 = files.posi8 + files.length8;
            pfile_position = &files.posi9;
            pfile_size = &files.length9;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SSTB1:
            files.posi10 = files.posi9 + files.length9;
            pfile_position = &files.posi10;
            pfile_size = &files.length10;
            file_size = 0;
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;

        case GESTION_SM_TO_WRITE_SSTB3:
            files.posi11 = files.posi10 + files.length10;
            pfile_position = &files.posi11;
            pfile_size = &files.length11;
            file_size = 0;
            
            file_last_action = 1;
            
            gestion_state = GESTION_SM_GET_READY_FOR_CHUNK;
            break;
            
        case GESTION_SM_TO_MONITORING_LEAVE:
            timer_standby = 300;
            Display_ShowNumbers(DISPLAY_PROG);
            gestion_state = GESTION_SM_WAIT_TO_LEAVE;
            break;

        case GESTION_SM_WAIT_TO_LEAVE:
            if (!timer_standby)
            {
                Usart1Send((char *) "Leaving monitoring confirmed\r\n");
                gestion_state = GESTION_SM_INIT;
            }
            break;

        default:
            gestion_state = GESTION_SM_INIT;
            break;

        }

        Display_ShowNumbersAgain();

        // look careful this function can change the program state
        gestion_state = UpdateUart (gestion_state);
    }
}

//--- end of file ---//
