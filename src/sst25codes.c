//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SST25CODES.C ###################################
//------------------------------------------------------

// Includes --------------------------------------------------------------------
#include "sst25codes.h"
#include "sst25.h"
#include "flash_program.h"

#include "usart.h"
#include "tim.h"

// for new version 2
#include "memory_conf.h"

#include <stdio.h>


// Private Types Constants and Macros ------------------------------------------
const unsigned short v_offset_codes_in_sst [4] = { OFFSET_CODES_256,
                                                   OFFSET_CODES_512,
                                                   OFFSET_CODES_768,
                                                   OFFSET_CODES_1024 };


// Externals variables ---------------------------------------------------------
extern mem_bkp_typedef memory_backup;


// Globals ---------------------------------------------------------------------



// Module Private Functions ----------------------------------------------------
unsigned char SST_UpdateNewCode (unsigned int * p, unsigned short posi, unsigned int new_code);


// Module Functions ------------------------------------------------------------
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
    unsigned int base_page_address = 0;
    unsigned short address_offset_in_page = 0;
    
    if (position < CODES_POSI_256)
    {
        base_page_address = OFFSET_CODES_256;
        address_offset_in_page = position;
    }
    else if (position < CODES_POSI_512)
    {
        base_page_address = OFFSET_CODES_512;
        address_offset_in_page = position - CODES_POSI_256;
    }
    else if (position < CODES_POSI_768)
    {
        base_page_address = OFFSET_CODES_768;
        address_offset_in_page = position - CODES_POSI_512;
    }
    else if (position < CODES_POSI_1024)
    {
        base_page_address = OFFSET_CODES_1024;
        address_offset_in_page = position - CODES_POSI_768;
    }
    else
        return FAILED;

    // page bkp -- 1kByte
    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, base_page_address);

    // page erase -- 4kBytes
    Clear4KNVM(base_page_address);
    
    // update 4bytes code on 1kByte backuped page
    if (SST_UpdateNewCode(memory_backup.v_bkp, address_offset_in_page, new_code) == FAILED)
        return FAILED;
    
    // write back 1kBytes from backuped page
    writeBufferNVM ((unsigned char *) memory_backup.v_bkp_8u, 1024, base_page_address);
            
    return PASSED;
}


unsigned char SST_UpdateNewCode (unsigned int * p, unsigned short posi, unsigned int new_code)
{
    if (posi > 255)
        return FAILED;

    *(p + posi ) = new_code;

    return PASSED;
}


//Answers between 0 - 1023 if the control-code is already in SST Memory, else answers 0xFFFF
unsigned short SST_CheckBaseMask (unsigned int code_to_find, unsigned int mask)
{
    unsigned int * p_mem;

    code_to_find = code_to_find & mask;

    for (unsigned char i = 0; i < 4; i++)
    {
        //cargo el segmento de memoria
        readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, v_offset_codes_in_sst[i]);
        p_mem = memory_backup.v_bkp;

        for (unsigned short j = 0; j < 256; j++)
        {
            if (code_to_find == (*p_mem & mask))
                return (j + CODES_POSI_256 * i);
            
            p_mem++;
        }
        
    }

    return 0xffff;
}


//Answers 0 if not button, else 1 to 4 
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


//answers with the data in memory position of the index, even if its empty
unsigned int SST_CheckIndexInMemory (unsigned short index_to_find)
{
    unsigned int * p_mem;

    unsigned int base_page_address = 0;
    unsigned short address_offset_in_page = 0;
    
    if (index_to_find < CODES_POSI_256)
    {
        base_page_address = OFFSET_CODES_256;
        address_offset_in_page = index_to_find;
    }
    else if (index_to_find < CODES_POSI_512)
    {
        base_page_address = OFFSET_CODES_512;
        address_offset_in_page = index_to_find - CODES_POSI_256;
    }
    else if (index_to_find < CODES_POSI_768)
    {
        base_page_address = OFFSET_CODES_768;
        address_offset_in_page = index_to_find - CODES_POSI_512;
    }
    else if (index_to_find < CODES_POSI_1024)
    {
        base_page_address = OFFSET_CODES_1024;
        address_offset_in_page = index_to_find - CODES_POSI_768;
    }
    else
        return 0xffffffff;


    readBufNVM8u((unsigned char *) memory_backup.v_bkp_8u, 1024, base_page_address);
    p_mem = memory_backup.v_bkp;

    return *(p_mem + address_offset_in_page);
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
