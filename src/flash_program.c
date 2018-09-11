/**
  *****************************************************************************
  * @title   FLASH_Program.c
  * @author  CooCox
  * @date    31 Oct 2012
  * @brief   This example provides a description of how to program the STM32F
  *          10x FLASH.
  *          the result of the programming operation is stored in
  *          to the MemoryProgramStatus variable.
  *          Four LEDs are toggled if t
  *          he flash program has been down.
  *          This example has been tested on
  *           KEIL MCBSTM32 board, STM32F103RBT6 device.
  *******************************************************************************
  */
////// The above comment is automatically generated by CoIDE ///////////////////

#include "flash_program.h"
#include "stm32f0xx_flash.h"
#include "sst25.h"

//desde que saque main.h
#include "usart.h"
#include "tim.h"
#include <stdio.h>
//desde que saque main.h


/* Externals variables ---------------------------------------------------------*/
#ifdef CONFIGURATION_IN_FLASH
extern unsigned char b1t;
extern unsigned char b1r;
extern unsigned char b2t;
extern unsigned char b2r;
extern unsigned char b3t;
extern unsigned char b3r;
extern unsigned char b4t;
extern unsigned char b4r;

extern parameters_typedef param_struct;
#endif

#ifdef CONFIGURATION_IN_SST
extern parameters_typedef param_struct;
#endif

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/



extern mem_bkp_typedef memory_backup;
extern filesystem_typedef files;
//extern unsigned int v_bkp [];

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//const int vmem [1024] ;
unsigned int * pmem = (unsigned int *)0x08007000;	//Sector 7 Page 28


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void Load16SamplesShort (unsigned short * buf, unsigned int posi)
{
	/*
	unsigned char i;
	unsigned char int_buf [32];
	unsigned short * p_memory;

	readBufNVM(int_buf, 32, posi);
	p_memory = (unsigned short *) int_buf;

	for (i = 0; i < 16; i++)
	{
		*(buf + i) =  *(p_memory + i);
	}
	*/
	readBufNVM8u((unsigned char *) buf, 32, posi);
}

void Load16SamplesChar (unsigned char * buf, unsigned int posi)
{
	readBufNVM8u(buf, 16, posi);
}

void ShowFileSystem(void)
{
	char str [100];

	  Usart1Send((char *) "File System:\r\n");
	  Wait_ms(100);
	  //LoadFilesIndex();

	  sprintf(str, "num0: %08x %08x %d\r\n", files.posi0, files.lenght0, files.lenght0);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num1: %08x %08x %d\r\n", files.posi1, files.lenght1, files.lenght1);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num2: %08x %08x %d\r\n", files.posi2, files.lenght2, files.lenght2);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num3: %08x %08x %d\r\n", files.posi3, files.lenght3, files.lenght3);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num4: %08x %08x %d\r\n", files.posi4, files.lenght4, files.lenght4);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num5: %08x %08x %d\r\n", files.posi5, files.lenght5, files.lenght5);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num6: %08x %08x %d\r\n", files.posi6, files.lenght6, files.lenght6);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num7: %08x %08x %d\r\n", files.posi7, files.lenght7, files.lenght7);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num8: %08x %08x %d\r\n", files.posi8, files.lenght8, files.lenght8);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "num9: %08x %08x %d\r\n", files.posi9, files.lenght9, files.lenght9);
	  Usart1Send(str);
	  Wait_ms(100);
}

void LoadFilesIndex (void)
{
	unsigned char posi [80];
	unsigned int * p_memory;

	readBufNVM8u(posi, 80, OFFSET_FILESYSTEM);
	p_memory = (unsigned int *) posi;

	//num0
	files.posi0 = *p_memory;
	files.lenght0 = *(p_memory + 1);	//probar asi o incrementando el puntero

	//num1
	files.posi1 = *(p_memory + 2);
	files.lenght1 = *(p_memory + 3);

	//num2
	files.posi2 = *(p_memory + 4);
	files.lenght2 = *(p_memory + 5);

	//num3
	files.posi3 = *(p_memory + 6);
	files.lenght3 = *(p_memory + 7);

	//num4
	files.posi4 = *(p_memory + 8);
	files.lenght4 = *(p_memory + 9);

	//num5
	files.posi5 = *(p_memory + 10);
	files.lenght5 = *(p_memory + 11);

	//num6
	files.posi6 = *(p_memory + 12);
	files.lenght6 = *(p_memory + 13);

	//num7
	files.posi7 = *(p_memory + 14);
	files.lenght7 = *(p_memory + 15);

	//num8
	files.posi8 = *(p_memory + 16);
	files.lenght8 = *(p_memory + 17);

	//num9
	files.posi9 = *(p_memory + 18);
	files.lenght9 = *(p_memory + 19);
}

#ifdef CONFIGURATION_IN_SST
void LoadConfiguration (void)
{
	unsigned char posi [80];
	//unsigned int * p_memory;

	readBufNVM8u(posi, 80, OFFSET_CONFIGURATION);
	//p_memory = (unsigned int *) posi;

	//param_struct.b1t = *p_memory;
	//param_struct.b1r = *(p_memory + 1);
	//param_struct.b2t = *(p_memory + 2);
	//param_struct.b2r = *(p_memory + 3);
	//param_struct.b3t = *(p_memory + 4);
	//param_struct.b3r = *(p_memory + 5);
	//param_struct.b4t = *(p_memory + 6);
	//param_struct.b4r = *(p_memory + 7);

	param_struct.b1t = *posi;
	param_struct.b1r = *(posi + 1);
	param_struct.b2t = *(posi + 2);
	param_struct.b2r = *(posi + 3);
	param_struct.b3t = *(posi + 4);
	param_struct.b3r = *(posi + 5);
	param_struct.b4t = *(posi + 6);
	param_struct.b4r = *(posi + 7);

	//ahora los tiempos internos
	//param_struct.interdigit = *(unsigned short *) (posi + 8);
	//param_struct.wait_for_code = *(unsigned short *) (posi + 10);

	//ahora los tiempos internos	TODO: por ahora no se configuran
	param_struct.interdigit = 3000;
        //modificacion 09-09-18 doy mas tiempo para recibir un codigo en programacion
	// param_struct.wait_for_code = 10000;
	param_struct.wait_for_code = 60000;        

}
#endif

void ShowConfiguration(void)
{
	char str [100];

	  Usart1Send((char *) "Configuration:\r\n");
#ifdef CONFIGURATION_IN_SST
	  LoadFilesIndex();

	  sprintf(str, "B1T: %d, B1R: %d\r\n", param_struct.b1t, param_struct.b1r);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "B2T: %d, B2R: %d\r\n", param_struct.b2t, param_struct.b2r);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "B3T: %d, B3R: %d\r\n", param_struct.b3t, param_struct.b3r);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "B4T: %d, B4R: %d\r\n", param_struct.b4t, param_struct.b4r);
	  Usart1Send(str);
	  Wait_ms(100);
#endif

#ifdef CONFIGURATION_IN_FLASH
	  sprintf(str, "B1T: %d, B1R: %d\r\n", b1t, b1r);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "B2T: %d, B2R: %d\r\n", b2t, b2r);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "B3T: %d, B3R: %d\r\n", b3t, b3r);
	  Usart1Send(str);
	  Wait_ms(100);

	  sprintf(str, "B4T: %d, B4R: %d\r\n", b4t, b4r);
	  Usart1Send(str);
	  Wait_ms(100);
#endif
}

void UpdateFileIndex (unsigned char numfile, unsigned int index, unsigned int lenght)
{
	unsigned int * p_memory;

	p_memory = (unsigned int *) &files.posi0;

	*(p_memory + numfile * 2) = index;
	*(p_memory + numfile * 2 + 1) = lenght;

}

void SaveFilesIndex (void)
{
	unsigned short * p_memory;


	//clearNVM();	//para pruebas
	//ojo la memoria debe estar borrada de antes
	p_memory = (unsigned short *) &files.posi0;
	//writeBufNVM(p_memory, sizeof(filesystem_typedef), 0);
	writeBufNVM16u(p_memory, sizeof(filesystem_typedef) >> 1, 0);
}

unsigned char ReadMem (void)
{
	if (*pmem == 0xffffffff)
		return 1;
	else
		return 0;
}

unsigned char EraseAllMemory_FLASH(void)
{
	unsigned char resp = 0;
	//borro toda la memoria desde pagina 28 a 31
	ErasePage(PAGE28,0);
	ErasePage(PAGE29,0);
	ErasePage(PAGE30,0);
	ErasePage(PAGE31,1);
	//reviso si borre
	if (*(unsigned int *) PAGE28 == 0xFFFFFFFF)
		resp++;

	if (*(unsigned int *) PAGE29 == 0xFFFFFFFF)
		resp++;

	if (*(unsigned int *) PAGE30 == 0xFFFFFFFF)
		resp++;

	if (*(unsigned int *) PAGE31 == 0xFFFFFFFF)
		resp++;

	if (resp == 4)
		return PASSED;
	else
		return FAILED;
}

/*
unsigned short Look_Code_In_Memory(unsigned int code_to_look_for)
{
	unsigned short i;

	for (i = 0; i < FLASH_PAGE_SIZE; i++)
	{
		if (*(unsigned int*) (PAGE28_START + i) == code_to_look_for)
			return i;
	}

	return 0xFFFF;
}
*/

unsigned char Write_Code_To_Memory_FLASH(unsigned short position, unsigned int new_code)
{
	//check if possition is good
	if (position > PAGE31_END)
		return FAILED;

	if (position >= PAGE31_START)
	{
		BackupPage(memory_backup.v_bkp, (unsigned int *) PAGE31);
		ErasePage(PAGE31,0);
		if (UpdateNewCode(memory_backup.v_bkp, position - PAGE31_START, new_code) == FAILED)
			return FAILED;

		if (WritePage(memory_backup.v_bkp, PAGE31, 1) == FAILED)
			return FAILED;
	}
	else if (position >= PAGE30_START)
	{
		BackupPage(memory_backup.v_bkp, (unsigned int *) PAGE30);
		ErasePage(PAGE30,0);
		if (UpdateNewCode(memory_backup.v_bkp, position - PAGE30_START, new_code) == FAILED)
			return FAILED;

		if (WritePage(memory_backup.v_bkp, PAGE30, 1) == FAILED)
			return FAILED;
	}
	else if (position >= PAGE29_START)
	{
		BackupPage(memory_backup.v_bkp, (unsigned int *) PAGE29);
		ErasePage(PAGE29,0);
		if (UpdateNewCode(memory_backup.v_bkp, position - PAGE29_START, new_code) == FAILED)
			return FAILED;

		if (WritePage(memory_backup.v_bkp, PAGE29, 1) == FAILED)
			return FAILED;
	}
	else if (position >= PAGE28_START)
	{
		BackupPage(memory_backup.v_bkp, (unsigned int *) PAGE28);
		ErasePage(PAGE28,0);
		if (UpdateNewCode(memory_backup.v_bkp, position, new_code) == FAILED)
			return FAILED;

		if (WritePage(memory_backup.v_bkp, PAGE28, 1) == FAILED)
			return FAILED;
	}

	return PASSED;
}
/**
  * @brief    After Reset, the Flash memory Program/Erase Controller is locked.
  * To unlock it, the FLASH_Unlock function is used. Before programming the
  * desired addresses, an erase operation is performed using the flash erase
  * page feature. The erase procedure starts with the calculation of the number
  * of pages to be used. Then all these pages will be erased one by one by
  * calling FLASH_ErasePage function.
  * Once this operation is finished, the programming operation will be performed by
  * using the FLASH_ProgramWord function. The written data is then checked and the
  * result of the programming operation is stored into the MemoryProgramStatus variable.
  * @param  None
  * @retval None
  */

void BackupPage(unsigned int * p, unsigned int * page_addr)
{
	unsigned short i;

	for (i = 0; i < 255; i++)
	{
		*(p + i) = *(page_addr + i);
	}
}

void ErasePage(uint32_t p_addr, unsigned char with_lock)
{
	FLASH_Unlock();
	FLASH_ErasePage(p_addr);
	if (with_lock)
		FLASH_Lock();
}

unsigned char UpdateNewCode(unsigned int * p, unsigned short posi, unsigned int new_code)
{
	if (posi > 255)
		return FAILED;

	*(p + posi ) = new_code;

	return PASSED;
}

unsigned char WritePage(unsigned int * p, uint32_t p_addr, unsigned char with_lock)
{
	unsigned short i;
	unsigned int * p_verif;

	p_verif = (unsigned int *) p_addr;

	for (i = 0; i < 255; i++)
	{
		FLASH_ProgramWord(p_addr, *(p + i));
		p_addr += 4;
	}

	if (with_lock)
		FLASH_Lock();

	//verifico memoria
	for (i = 0; i < 255; i++)
	{
		if (*(p_verif + i) != *(p + i))
			return FAILED;
	}
	return PASSED;
}

#ifdef CONFIGURATION_IN_FLASH
unsigned char WriteConfigurations (void)
{
	unsigned char * p_dos;
	//hago bkp de la pagina??? SI SI NO SE TRABA EL PROGRAMA
	BackupPage(memory_backup.v_bkp, (unsigned int *) PAGE27);

	ErasePage(PAGE27,0);

	//update en memoria
	p_dos = (unsigned char *) &memory_backup.v_bkp[0];

	*p_dos = param_struct.b1t;
	*(p_dos + 1) = param_struct.b1r;
	*(p_dos + 2) = param_struct.b2t;
	*(p_dos + 3) = param_struct.b2r;
	*(p_dos + 4) = param_struct.b3t;
	*(p_dos + 5) = param_struct.b3r;
	*(p_dos + 6) = param_struct.b4t;
	*(p_dos + 7) = param_struct.b4r;


	//faltan los tiempos internos



	if (WritePage(memory_backup.v_bkp, PAGE27, 1) == FAILED)
		return FAILED;

	return PASSED;
}
#endif
