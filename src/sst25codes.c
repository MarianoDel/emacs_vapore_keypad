//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SST25CODES.C ###################################
//------------------------------------------------------

/* Includes ------------------------------------------------------------------*/
#include "sst25codes.h"
#include "sst25.h"
#include "flash_program.h"

#include "usart.h"
#include "tim.h"

#include <stdio.h>



/* Externals variables ---------------------------------------------------------*/
extern mem_bkp_typedef memory_backup;


/* Global variables ------------------------------------------------------------*/



//---------------------Module Functions --------------------------------
void SST_MemoryDump (unsigned int address)
{
    unsigned char i;
    char str [64];
    unsigned short * p_mem_dump;

    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, address);
    p_mem_dump = (unsigned short *) memory_backup.v_bkp;

    for (i = 0; i < 6; i++)		//saco 6 renglones
    {
        //leo la memoria y la voy enviando al puerto serie
        sprintf(str, "\r\n%04X %04X %04X %04X %04X %04X %04X %04X", *(p_mem_dump), *(p_mem_dump + 1),
                *(p_mem_dump + 2), *(p_mem_dump + 3),
                *(p_mem_dump + 4), *(p_mem_dump + 5),
                *(p_mem_dump + 6), *(p_mem_dump + 7));
        p_mem_dump += 8;

        Usart1Send(str);
        Wait_ms(200);
    }
}

//Write New control-code into the SST Memory
unsigned char SST_WriteCodeToMemory (unsigned short position, unsigned int new_code)
{
    if (position < CODES_POSI_256)
    {
        //backup de la pagina
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_256);
        //borro pagina
        Clear4KNVM(OFFSET_CODES_256);

        if (UpdateNewCode(memory_backup.v_bkp, position, new_code) == FAILED)
            return FAILED;

        //writeBufNVM((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_256);
        writeBufNVM16u((unsigned short *) memory_backup.v_bkp_16u, 512, OFFSET_CODES_256);
    }
    else if (position < CODES_POSI_512)
    {
        //backup de la pagina
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_512);
        //borro pagina
        Clear4KNVM(OFFSET_CODES_512);

        if (UpdateNewCode(memory_backup.v_bkp, (position - CODES_POSI_256), new_code) == FAILED)
            return FAILED;

        //writeBufNVM((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_512);
        writeBufNVM16u((unsigned short *) memory_backup.v_bkp_16u, 512, OFFSET_CODES_512);
    }
    else if (position < CODES_POSI_768)
    {
        //backup de la pagina
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_768);
        //borro pagina
        Clear4KNVM(OFFSET_CODES_768);

        if (UpdateNewCode(memory_backup.v_bkp, (position - CODES_POSI_512), new_code) == FAILED)
            return FAILED;

        writeBufNVM16u((unsigned short *) memory_backup.v_bkp_16u, 1024, OFFSET_CODES_768);
    }
    else if (position < CODES_POSI_1024)
    {
        //backup de la pagina
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_1024);
        //borro pagina
        Clear4KNVM(OFFSET_CODES_1024);

        if (UpdateNewCode(memory_backup.v_bkp, (position - CODES_POSI_768), new_code) == FAILED)
            return FAILED;

        writeBufNVM16u((unsigned short *) memory_backup.v_bkp_16u, 1024, OFFSET_CODES_1024);
    }
    else
        return FAILED;

    return PASSED;
}

//Answers between 0 - 1023 if the control-code is already in SST Memory, else answers 0xFFFF
unsigned short SST_CheckBaseMask (unsigned int code_to_find, unsigned int mask)
{
    unsigned short i;
    unsigned int * p_mem;
    //char str [64];

    code_to_find = code_to_find & mask;

    //cargo 256 numeros al vector de 1024 posiciones
    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_256);
    p_mem = memory_backup.v_bkp;

    //sprintf(str, "\r\ncomparo %08X %08X", *p_mem, code_to_find);
    //Usart1Send(str);
    //Wait_ms(200);
    //if ((*p_mem & mask) == code_to_find)
    //	return 1;


    for (i = 0; i < 256; i++)
    {
        if (code_to_find == (*p_mem & mask))
            return i;
        p_mem++;
    }


    //no aparecio cargo 1024 posiciones mas
    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_512);
    p_mem = memory_backup.v_bkp;

    for (i = 0; i < 256; i++)
    {
        if (code_to_find == (*p_mem & mask))
            return i + CODES_POSI_256;
        p_mem++;
    }

    //no aparecio cargo 1024 posiciones mas
    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_768);
    p_mem = memory_backup.v_bkp;

    for (i = 0; i < 256; i++)
    {
        if (code_to_find == (*p_mem & mask))
            return i + CODES_POSI_512;
        p_mem++;
    }

    //no aparecio cargo 1024 posiciones mas
    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_1024);
    p_mem = memory_backup.v_bkp;

    for (i = 0; i < 256; i++)
    {
        if (code_to_find == (*p_mem & mask))
            return i + CODES_POSI_768;
        p_mem++;
    }

    return 0xffff;

}

//0 sino existe el boton sino de 1 a 4
unsigned char SST_CheckButton (unsigned int code_to_check, unsigned int button_mask)
{
#ifdef EV1527_BUTTONS
    code_to_check &= button_mask;

    if (code_to_check == 0x04)			//B1
    {
        return 1;
    }
    else if (code_to_check == 0x08)		//B2
    {
        return 2;
    }
    else if (code_to_check == 0x10)		//B3
    {
        return 3;
    }
    else if (code_to_check == 0x02)		//B4
    {
        return 4;
    }
    else
        return 0;
#endif

#ifdef HT6P20B2_BUTTONS
    code_to_check >>= 4;
    code_to_check &= button_mask;

    //reviso botones nuevos y viejos

#ifdef B1_BY_B2
    if ((code_to_check == 4) || (code_to_check == 0x0C))
    {
        return 1;
    }
    else if ((code_to_check == 2) || (code_to_check == 0x0A))
    {
        return 2;
    }
    else if ((code_to_check == 6) || (code_to_check == 0x0E))
    {
        return 3;
    }
    else if (code_to_check == 8)
    {
        return 4;
    }
    else
        return 0;
#else
    if ((code_to_check == 2) || (code_to_check == 0x0C))
    {
        return 1;
    }
    else if ((code_to_check == 4) || (code_to_check == 0x0A))
    {
        return 2;
    }
    else if ((code_to_check == 6) || (code_to_check == 0x0E))
    {
        return 3;
    }
    else if (code_to_check == 8)
    {
        return 4;
    }
    else
        return 0;
#endif
#endif

    /*
      if (code_to_check == 2)
      {
      return 1;
      }
      else if (code_to_check == 4)
      {
      return 2;
      }
      else if (code_to_check == 6)
      {
      return 3;
      }
      else if (code_to_check == 8)
      {
      return 4;
      }
      else
      return 0;
    */
}

unsigned int SST_CheckIndexInMemory (unsigned short index_to_find)
{
    unsigned int * p_mem;

    if (index_to_find < 256)
    {
        //cargo 256 numeros al vector de 1024 posiciones
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_256);
        p_mem = memory_backup.v_bkp;

        return *(p_mem + index_to_find);
    }
    else if (index_to_find < 512)
    {
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_512);
        p_mem = memory_backup.v_bkp;

        return *(p_mem + index_to_find - 256);
    }
    else if (index_to_find < 768)
    {
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_768);
        p_mem = memory_backup.v_bkp;

        return *(p_mem + index_to_find - 512);
    }
    else if (index_to_find < 1024)
    {
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, OFFSET_CODES_1024);
        p_mem = memory_backup.v_bkp;

        return *(p_mem + index_to_find - 768);
    }
    else
        return 0xffffffff;
}


unsigned char SST_EraseAllMemory (void)
{
    unsigned char resp = 0;
    unsigned char check = 0;

    //la memoria SST en la parte de los codigos
    Clear4KNVM(OFFSET_CODES_256);
    Clear4KNVM(OFFSET_CODES_512);
    Clear4KNVM(OFFSET_CODES_768);
    Clear4KNVM(OFFSET_CODES_1024);

    //reviso si borre
    readBufNVM8u(&check, 1, OFFSET_CODES_256);
    if (check == 0xFF)
        resp++;

    readBufNVM8u(&check, 1, OFFSET_CODES_512);
    if (check == 0xFF)
        resp++;


    readBufNVM8u(&check, 1, OFFSET_CODES_768);
    if (check == 0xFF)
        resp++;

    readBufNVM8u(&check, 1, OFFSET_CODES_1024);
    if (check == 0xFF)
        resp++;

    if (resp == 4)
        return PASSED;
    else
        return FAILED;

}

//--- end of file ---//
