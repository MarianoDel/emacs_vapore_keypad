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


/* Externals variables ---------------------------------------------------------*/
extern volatile unsigned short timer_standby;
extern volatile unsigned char binary_full;
extern mem_bkp_typedef memory_backup;
extern filesystem_typedef files;
extern unsigned char next_pckt;
extern unsigned char file_done;


/* Global variables ------------------------------------------------------------*/
gestion_sm_t gestion_state = GESTION_SM_INIT;
unsigned int file_size = 0;

//respuestas a los envios desde la PC
const char s_1024 [] = {"1024 bytes\r\n"};
const char s_1024_conf [] = {"1024 config bytes\r\n"};
const char s_ok [] = {"OK\r\n"};
const char s_ok_finish_conf [] = {"FINISH CONF\r\n"};


/* Private Module Functions ----------------------------------------------------*/


/* Module Functions ------------------------------------------------------------*/
void FuncGestion (void)
{
    USART1->CR1 &= ~USART_CR1_UE;
    USART1->BRR = USART_115200;
    USART1->CR1 |= USART_CR1_UE;

    Usart1Send((char *) "STM32F030K6 Programa de Gestion de Memoria\r\n");

    //cargo los valores de memoria
    if (readJEDEC() == 0)
    {
        while (1)
        {
            Usart1Send((char *) "Memoria MAL\r\n");
            Wait_ms(2000);
            Usart1Send((char *) "Memoria con JEDEC MAL\r\n");
            Wait_ms(2000);
        }
    }

    Usart1Send((char *) "Memoria con JEDEC OK\r\n");
    Wait_ms(100);

    ShowFileSystem();

#ifdef INFO_IN_SST
    LoadConfiguration();
    ShowConfiguration();
#endif

    ShowNumbers(DISPLAY_PROG);
    
    while (1)
    {
        switch (gestion_state)
        {
        case GESTION_SM_INIT:

            gestion_state = GESTION_SM_MAIN;
            //binary_bytes = 1024;
            // current_size = 0;

            break;

        case GESTION_SM_TO_MONITORING:
            timer_standby = 50;

            Usart1Send((char *) "Monitoring on 115200 confirmed\r\n");
            ShowNumbers(DISPLAY_REMOTE);

            LoadConfiguration();
            gestion_state = GESTION_SM_TO_MONITORINGA;
            break;

        case GESTION_SM_TO_MONITORINGA:
            if (!timer_standby)
            {
                if (readJEDEC())
                    Usart1Send((char *) "Memoria OK\r\n");
                else
                    Usart1Send((char *) "Memoria MAL\r\n");

                gestion_state = GESTION_SM_TO_MONITORINGB;
                timer_standby = 50;
            }
            break;

        case GESTION_SM_TO_MONITORINGB:
            if (!timer_standby)
            {
                if (readNVM(0) != 0xFF)
                    Usart1Send((char *) "Parece grabado el Filesystem\r\n");
                else
                    Usart1Send((char *) "Parece no tener Filesystem\r\n");

                timer_standby = 50;
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_MONITORINGB2:
            if (!timer_standby)
            {
                if (readNVM(OFFSET_CONFIGURATION) != 0xFF)
                    Usart1Send((char *) "Parece tener configuracion\r\n");
                else
                    Usart1Send((char *) "Parece no tener configuracion\r\n");

                timer_standby = 50;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_MONITORINGC:
            break;

        case GESTION_SM_MAIN:

            break;

        case GESTION_SM_TO_MAIN_TIMEOUT:
            Usart1Send((char *) "Timeout\r\n");
            gestion_state = GESTION_SM_INIT;
            timer_standby = 300;	//espero que se limpien las teclas
            break;

        case GESTION_SM_TO_FLUSH_SST:
            if (readJEDEC())
            {
                Usart1Send((char *) "Memoria OK\r\n");
                clearNVM();
                Usart1Send((char *) "Borrada Completa\r\n");
            }
            else
            {
                Usart1Send((char *) "Memoria MAL\r\n");
            }
            gestion_state = GESTION_SM_TO_MONITORINGC;
            break;

        case GESTION_SM_TO_WRITE_SST_CONF:
            if (readJEDEC())
            {
                Usart1Send((char *) "Memoria OK\r\n");
                Clear4KNVM(OFFSET_CONFIGURATION);
                Usart1Send((char *) "Borrada 4K\r\n");
                gestion_state++;
                timer_standby = 50;
            }
            else
            {
                Usart1Send((char *) "Memoria MAL\r\n");
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST_CONFA:
            if (!timer_standby)
            {
                Usart1Send((char *) s_1024_conf);
                UsartRxBinary();
                gestion_state++;
            }
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
            gestion_state = GESTION_SM_TO_MONITORINGC;
            break;

        case GESTION_SM_TO_WRITE_SST0:
            if (readJEDEC())
            {
                Usart1Send((char *) "Memoria OK\r\n");
                clearNVM();
                files.posi0 = OFFSET_FIRST_FILE;
                file_size = 0;
                gestion_state++;
            }
            else
            {
                Usart1Send((char *) "Memoria MAL\r\n");
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST0_A:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST0_B:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi0 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST0_C:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST0_A;
            }


            if (file_done)
            {
                file_done = 0;
                files.lenght0 = file_size;
                files.posi1 = files.posi0 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }

            break;

        case GESTION_SM_TO_WRITE_SST1:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST1_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi1 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi1 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi1 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	  //termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST1_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST1;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght1 = file_size;
                files.posi2 = files.posi1 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST2:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST2_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi2 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi2 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi2 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST2_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST2;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght2 = file_size;
                files.posi3 = files.posi2 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST3:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST3_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi3 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi3 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi3 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST3_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST3;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght3 = file_size;
                files.posi4 = files.posi3 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST4:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST4_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi4 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi4 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi4 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST4_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST4;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght4 = file_size;
                files.posi5 = files.posi4 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST5:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST5_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi5 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi5 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi5 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST5_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST5;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght5 = file_size;
                files.posi6 = files.posi5 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST6:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST6_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi6 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi6 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi6 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST6_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST6;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght6 = file_size;
                files.posi7 = files.posi6 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST7:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST7_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi7 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi7 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi7 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST7_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST7;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght7 = file_size;
                files.posi8 = files.posi7 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST8:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST8_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi8 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi8 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi8 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST8_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST8;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght8 = file_size;
                files.posi9 = files.posi8 + file_size;
                file_size = 0;
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_WRITE_SST9:
            Usart1Send((char *) s_1024);
            UsartRxBinary();
            gestion_state++;
            break;

        case GESTION_SM_TO_WRITE_SST9_A:	//me quedo esperando completar el buffer
            if (binary_full)
            {
                //writeBufNVM(memory_backup.v_bkp_8u, 1024, files.posi9 + file_size);
                //writeBufNVM8u(memory_backup.v_bkp_8u, 1024, files.posi9 + file_size);
                writeBufNVM16u(memory_backup.v_bkp_16u, 512, files.posi9 + file_size);
                file_size += 1024;
                Usart1Send((char *) s_ok);	//termine de recibir 1024
                gestion_state++;
            }
            break;

        case GESTION_SM_TO_WRITE_SST9_B:	//espero proximo paquete o terminar (terminar me saca solo)
            if (next_pckt)
            {
                next_pckt = 0;
                gestion_state = GESTION_SM_TO_WRITE_SST9;
            }

            if (file_done)
            {
                file_done = 0;
                files.lenght9 = file_size;
                file_size = 0;
                Usart1Send((char *) "File system grabado\r\n");
                SaveFilesIndex();
                gestion_state = GESTION_SM_TO_MONITORINGC;
            }
            break;

        case GESTION_SM_TO_MONITORING_LEAVE:
            Usart1Send((char *) "Leaving monitoring confirmed\r\n");
            timer_standby = 300;	//espero que se limpien los buffers
            ShowNumbers(DISPLAY_PROG);
            gestion_state++;
            break;

        case GESTION_SM_TO_MONITORINGE:
            if (!timer_standby)
            {
                gestion_state = GESTION_SM_TO_MAIN_OK;
            }
            break;

        case GESTION_SM_TO_SAVE_TIME:
            gestion_state = GESTION_SM_TO_MONITORINGC;		//seguro que llegué desde MONITORINGC
            break;

        case GESTION_SM_TO_MAIN_WAIT_5SEGS:
            timer_standby = 5000;	//espero 5 segundos luego del codigo grabado OK
            gestion_state = GESTION_SM_TO_MAIN_WAIT_5SEGSA;
            break;

        case GESTION_SM_TO_MAIN_WAIT_5SEGSA:
            if (!timer_standby)
                gestion_state = GESTION_SM_TO_MAIN_OK;
            break;

        case GESTION_SM_TO_MAIN_OK:
            gestion_state = GESTION_SM_INIT;
            break;

        default:
            gestion_state = GESTION_SM_INIT;
            break;

        }

        ShowNumbersAgain();
        //ojo esta funcion puede cambiar el estado del programa de gestion
        gestion_state = UpdateUart (gestion_state);
    }
}

//--- end of file ---//
