/**
  ******************************************************************************
  * @file    Template_2/main.c
  * @author  Nahuel
  * @version V1.0
  * @date    22-August-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Use this template for new projects with stm32f0xx family.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx.h"
#include "hard.h"
#include "spi.h"
#include "sst25.h"
#include "comm.h"

#include "core_cm0.h"
#include "rws317.h"
#include "flash_program.h"
#include "sst25codes.h"
#include "usart.h"

//desde que saque main.h

#include "stm32f0x_gpio.h"
// #include "stm32f0x_uart.h"
#include <stdio.h>
#include <string.h>
#include "it.h"

// #include "stm32f0xx_spi.h"
// #include "stm32f0xx_rcc.h"

//fin desde que saque main.h

//#include <stdio.h>
//#include <string.h>




/* Externals ------------------------------------------------------------------*/
// ------- Externals para timers -------
volatile unsigned char timer_1seg = 0;
volatile unsigned short timer_standby = 0;
volatile unsigned short timer_led_comm = 0;
volatile unsigned short timer_keypad_enabled = 0;
volatile unsigned short wait_ms_var = 0;


// ------- Externals del Puerto serie  -------
volatile unsigned char tx1buff[SIZEOF_DATA];
volatile unsigned char rx1buff[SIZEOF_DATA];
volatile unsigned char usart1_have_data = 0;


mem_bkp_typedef memory_backup;
filesystem_typedef files;


unsigned char next_pckt = 0;
unsigned char file_done = 0;

volatile unsigned char pilot_code = 0;
volatile unsigned short code0 = 0;
volatile unsigned short code1 = 0;
volatile unsigned char errorcode = 0;

#ifdef CONFIGURATION_IN_FLASH
parameters_typedef __attribute__ ((section (".myStruct"))) const param_struct = {30, 5, 30, 5, 30, 5, 30, 5, 3000, 10000};

unsigned char b1t;
unsigned char b1r;
unsigned char b2t;
unsigned char b2r;
unsigned char b3t;
unsigned char b3r;
unsigned char b4t;
unsigned char b4r;

#endif

#ifdef CONFIGURATION_IN_SST
//parameters_typedef param_struct = {30, 5, 30, 5, 30, 5, 30, 5, 3000, 10000};
parameters_typedef param_struct;
#endif
//parameters_typedef * p_params;



volatile short v_samples1[16];
volatile short v_samples2[16];
volatile unsigned char update_samples = 0;
volatile unsigned char buff_in_use = 1;

/* Globals ------------------------------------------------------------------*/
static __IO uint32_t TimingDelay;

unsigned short counter = TIM3_ARR;
unsigned char last_digit = 0;
unsigned char last_internals = 0;
unsigned char buzzer_state = 0;
unsigned char buzz_multiple = 0;
unsigned char siren_state = 0;
unsigned short freq_us = 0;
unsigned char siren_steps = 0;

unsigned char display_state = 0;
unsigned char numbers[LAST_NUMBER];
unsigned char * p_numbers;

volatile unsigned char binary_full = 0;
volatile unsigned char switches_internals_timeout = 0;
volatile unsigned short wait_for_code_timeout;
volatile unsigned short interdigit_timeout;
volatile unsigned short buzzer_timeout;
volatile unsigned short display_timeout;
volatile unsigned short siren_timeout;

volatile unsigned char button_timer;
volatile unsigned short button_timer_internal;

//--- Respecto del KeyPad
unsigned char keypad_state = 0;
unsigned char remote_keypad_state = 0;
unsigned char unlock_by_remote = 0;

//--- Respecto de las alarmas y botones
//unsigned short last_act = 0;
unsigned short last_one_or_three = 0;
unsigned short last_two = 0;
unsigned char alarm_state = 0;
unsigned char repetition_counter = 0;

//--- Audio Global Variables
#define FILE_OFFSET 44
unsigned char audio_state = 0;
unsigned char numbers_speak[LAST_NUMBER_SPEAK];
unsigned char * p_numbers_speak;
unsigned int * p_files_addr;
unsigned int * p_files_lenght;
unsigned int current_size = 0;

//--- Para revisar la bateria
#ifdef CON_MODIFICACION_DIODO_BATERIA
volatile unsigned char timer_battery = 0;
unsigned short ac_counter = 0;
unsigned short bat_counter = 0;
unsigned char slow_toggle = 0;
#endif

const char s_1024 [] = {"1024 bytes\r\n"};
const char s_ok [] = {"OK\r\n"};

//const char s_bienvenida [] = {"Prueba RS232 STM32F030K6\r\n"};
//SPI_InitTypeDef  SPI_InitStructure;

/* Module Functions -----------------------------------------------------------*/
void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);

unsigned char FuncAlarm (void);
//unsigned char FuncProgCod (unsigned char);



// ------- de los switches ---
unsigned char ReadSwitches (void);
unsigned char ReadSwitches_Internals (void);
unsigned char CheckKeypad (unsigned char *, unsigned char *, unsigned char *, unsigned short *);
unsigned char CheckRemoteKeypad (unsigned char *, unsigned char *, unsigned char *, unsigned short *);
// ------- del display -------
void ConvertPositionToDisplay (unsigned short);
void UpdateDisplay (void);
void VectorToDisplay (unsigned char);
void ShowNumbers (unsigned char);
void SendSegment (unsigned char);
void ShowNumbersAgain (void);
// ------- del buzzer --------
void BuzzerCommands(unsigned char, unsigned char);
void UpdateBuzzer (void);
// ------- de la sirena ------
void SirenCommands(unsigned char);
void UpdateSiren (void);
// ------- del audio -------
void UpdateAudio (void);
void VectorToSpeak (unsigned char);
void PositionToSpeak(unsigned short);
// ------- para la bateria -------
void UpdateBattery (void);

//-------------------------------------------//
// @brief  Main program.
// @param  None
// @retval None
//------------------------------------------//
int main(void)
{
	unsigned char main_state = 0;
	unsigned short dummy16 = 0;
	//unsigned char i;	//, j;
#ifdef PROGRAMA_FACTORY_TEST
	unsigned char last_switches = 0;
#endif
	char str [40];

	unsigned short position, mass_erase_position;
	unsigned short code_position;
	unsigned int code;
	unsigned char switches, switches_posi0, switches_posi1, switches_posi2;
	//unsigned char switches_last;
	unsigned char result;
	//unsigned int file_size = 0;
	//unsigned short * p_mem_dump;
	//unsigned char button_remote = 0;
	unsigned char digit_remote = 0;

	//!< At this stage the microcontroller clock setting is already configured,
    //   this is done through SystemInit() function which is called from startup
    //   file (startup_stm32f0xx.s) before to branch to application main.
    //   To reconfigure the default setting of SystemInit() function, refer to
    //   system_stm32f0xx.c file

	//GPIO Configuration.
	GPIO_Config();
	CE_OFF;
	WP_OFF;
	PS_OFF;
	OE_OFF;
	FPLUS_OFF;
	F12PLUS_OFF;
	F5PLUS_OFF;
	BUZZER_OFF;
	LED_OFF;

//SENSAR TEMPERATURA 	((OK))
//ENVIAR ONE WIRE		((OK))

	//ACTIVAR SYSTICK TIMER
	 if (SysTick_Config(48000))
	 {
		 while (1)	/* Capture error */
		 {
			 if (LED)
				 LED_OFF;
			 else
				 LED_ON;

			 for (dummy16 = 0; dummy16 < 65000; dummy16++)
			 {
				 asm("nop");
			 }
		 }
	 }


#ifndef PROGRAMA_FACTORY_TEST
	 Wait_ms(2000);
#endif

		//TIM Configuration.
	 	 //para audio y sirena
		Timer_1_Init();
		//para codigo
		RxCode();	//trabo la interrupcion
		Timer_2_Init();

		//Timer_3_Init();
		//Timer_4_Init();

		Update_TIM1_CH1(100);
		Power_Ampli_Disa();
		//UART configuration.
		//USART_Config();


	  /* SPI configuration ------------------------------------------------------*/
	  SPI_Config();		//activo sin interrupcion

	  /* USART configuration ------------------------------------------------------*/
          Usart1Config();    //activo con int priority 5

	  //Usart1Send((char *) (const char *) "Prueba RS232 STM32F030K6\r\n");
	  Usart1Send((char *) " - Kirno Technology - STM32F030K6\r\n");
	  Wait_ms(100);
	  Usart1Send((char *) "Sistema de Alarma ALERTA VECINAL\r\n");
	  Wait_ms(100);
#ifdef PROGRAMA_NORMAL
	  Usart1Send((char *) "Panel Normal\r\n");
#endif

#ifdef PROGRAMA_DE_BUCLE
	  Usart1Send((char *) "Panel con Programa para Bucle\r\n");
#endif

#ifdef PROGRAMA_FACTORY_TEST
	  Usart1Send((char *) "Panel con Programa de testeo en fabrica\r\n");
#endif

	  //--- EMPIEZO PROGRAMA DE PRUEBAS EN FABRICA ---//
#ifdef PROGRAMA_FACTORY_TEST
	  main_state = TEST_INIT;

	  while (1)
	  {
		  switch (main_state)
		  {
			  case TEST_INIT:
				  ShowNumbers(3);
				  BuzzerCommands(BUZZER_SHORT_CMD, 2);
				  TimingDelay = 900;
				  main_state++;
				  break;

			  case TEST_CHECK_BUZZER:
				  if ((!TimingDelay) && (buzzer_state == BUZZER_INIT)) //espero que termine de enviar el buzzer
				  {
					  main_state++;
				  }
				  break;

			  case TEST_CHECK_MEMORY_WRITE:
				  if (WriteCodeToMemory_SST(100, 0x5555) == PASSED)
				  {
					  TimingDelay = 900;
					  main_state++;
				  }
				  else
					  main_state = TEST_ERROR;

				  ShowNumbers(2);
//				  main_state++;
				  break;

			  case TEST_CHECK_MEMORY_READ:
				  if (!TimingDelay)			//grabo memoria
				  {
					  if (CheckIndexInMemory_SST(100) == 0x5555)
					  {
						  ShowNumbers(0);
						  TimingDelay = 900;
						  main_state++;
					  }
					  else
						  main_state = TEST_ERROR;

					  ShowNumbers(1);
				  }
				  break;

			  case TEST_CHECK_KEYPAD:
				  if (!TimingDelay)
				  {
					  switches = ReadSwitches();

					  if (switches == NO_KEY)
					  {
						  ShowNumbers(DISPLAY_NONE);
						  last_switches = switches;
					  }
					  else
					  {
						  if (last_switches != switches)
						  {
							  last_switches = switches;
							  if (switches == ZERO_KEY)
								  ShowNumbers(10);
							  else if (switches == STAR_KEY)
								  ShowNumbers(DISPLAY_SQR_UP);
							  else if (switches == POUND_KEY)
								  ShowNumbers(DISPLAY_SQR_DOWN);
							  else
							  {
								  ShowNumbers(switches);
							  }
							  BuzzerCommands(BUZZER_SHORT_CMD, 1);
						  }
					  }
				  }
				  break;

			  case TEST_ERROR:
				  if (!TimingDelay)			//me quedo trabado mostrando el error
				  {
					  BuzzerCommands(BUZZER_SHORT_CMD, 1);
					  TimingDelay = 700;
				  }
				  break;


			  default:
				  main_state = TEST_INIT;
				  break;
		  }

		  UpdateBuzzer();

	  }

#endif
	  //--- TERMINA PROGRAMA DE PRUEBAS EN FABRICA ---//


	  UpdateDisplay();	//para setear puntero
	  UpdateDisplay();
	  BuzzerCommands(BUZZER_LONG_CMD, 2);

	  p_numbers_speak = numbers_speak;	//seteo puntero

	  //Power_Ampli_Ena();
	  //Ampli_to_Sirena();
	  //SirenCommands(SIREN_MULTIPLE_UP_CMD);
	  //SirenCommands(SIREN_MULTIPLE_DOWN_CMD);

	  //cargo los valores de memoria
	  LoadFilesIndex();
	  ShowFileSystem();

	  //reviso si esta grabada la memoria
	  if (files.posi0 == 0xFFFFFFFF)
	  {
		  //memoria no grabada
		  ShowNumbers(DISPLAY_ERROR);
		  BuzzerCommands(BUZZER_LONG_CMD, 10);
		  while (1)
			  UpdateBuzzer();
	  }

#ifdef CONFIGURATION_IN_FLASH
		 b1t = param_struct.b1t;
		 b1r = param_struct.b1r;
		 b2t = param_struct.b2t;
		 b2r = param_struct.b2r;
		 b3t = param_struct.b3t;
		 b3r = param_struct.b3r;
		 b4t = param_struct.b4t;
		 b4r = param_struct.b4r;
#endif

#ifdef CONFIGURATION_IN_SST
	  LoadConfiguration();
	  //reviso configuracion de memoria
	  if (param_struct.b1t == 0xFF)
	  {
		  //memoria no configurada
		  ShowNumbers(DISPLAY_ERROR2);
		  BuzzerCommands(BUZZER_LONG_CMD, 10);
		  while (1)
			  UpdateBuzzer();
	  }
	  ShowConfiguration();
#endif

	  //si esta todo bien configurado prendo el led
	  LED_ON;

	  //Usart1Send(s_bienvenida);
	  //RxCode_Enable();

	  //--- PRUEBA USART
	  /*
	  Usart1Send((const char *) "Prueba RS232 STM32F030K6\r\n");
	  while (1)
	  {
		  //Usart1Send((const char *) "Funciona\r\n");
		  //USARTx->TDR = 'M';
		  //Wait_ms(1000);

		  if (buffrx_ready)
		  {
			  j = InterpretarMsg(pbuffrx);
			  buffrx_ready = 0;
			  if (j != ERROR)
				  Usart1Send((const char *) "OK\r\n");
		  }
	  }
	  */
  	  //--- FIN PRUEBA USART



		/*
		while(1)
		{
			PS_ON;
			Wait_ms(1);
			PS_OFF;
			Wait_ms(1);
		}
		*/

	  //--- PRUEBA DE SEGMENTOS
	  /*
	  while (1)
	  {
		  segment = 1;
		  i = 0;
		  do
		  {
			  seg_neg =  ~segment;
			  SendSegment(seg_neg);
			  segment <<= 1;
			  i++;
			  Wait_ms(1000);
		  }
		  while (i < 8);

		  Wait_ms(5000);
	  }
	  */
	  //--- FIN PRUEBA DE SEGMENTOS

	  //--- PRUEBA DE NUMEROS DEL 0 AL 9
	  /*
	  while (1)
	  {
			if (LED)
				LED_OFF;
			else
				LED_ON;

			for (i = 0; i < 11; i++)
			{
				ShowNumbers(i);
				Wait_ms(1000);

			}
			Wait_ms(5000);
	  }
	  */
	  //--- FIN PRUEBA DE NUMEROS DEL 0 AL 9

	  //--- PRUEBA DE NUMEROS DEL 0 AL 9 + TACT SWITCHES
	  /*
	  while (1)
	  {
		  switches = ReadSwitches();

		  if (switches != NO_KEY)
		  {
				LED_ON;
				ShowNumbers(switches);
		  }
		  else
		  {
			  //ShowNumbers(DISPLAY_NONE);
			  LED_OFF;
		  }
	  }
	*/
	  //--- FIN PRUEBA DE NUMEROS DEL 0 AL 9 + TACT SWITCHES

	  //--- PRUEBA DE CODIGOS
	  /*
	  SirenCommands(SIREN_MULTIPLE_UP_CMD);
	  while (1)
	  {
		  RxCode();
		  UpdateSiren();
	  }
	*/
	  //--- FIN PRUEBA DE CODIGOS


	  while (1)
	  {
		  switch (main_state)
		  {
			  case MAIN_INIT:

				  switches_posi0 = 0;
				  switches_posi1 = 0;
				  switches_posi2 = 0;
				  position = 0;
				  //code = 0;

#ifdef CON_BLOQUEO_DE_KEYPAD
				  if (!interdigit_timeout)
					  main_state = MAIN_MAIN;
#else
				  main_state = MAIN_UNLOCK;
#endif
				  break;

			  case MAIN_MAIN:
				  //si termine alguna operacion y estaba con unlock
				  if (timer_keypad_enabled)
					  main_state = MAIN_TO_UNLOCK;

				  if (CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position) == KNUMBER_FINISH)
				  {
					  if (position == 951)
					  {
						  Usart1Send("Master Unlock\r\n");
						  main_state = MAIN_TO_UNLOCK;
					  }
					  else
					  {
						  if ((unsigned short) CheckIndexInMemory_SST(1000) == 0xFFFF)
						  {
							  if (position == 416)
							  {
								  Usart1Send("User default Unlock\r\n");
								  main_state = MAIN_TO_UNLOCK;
							  }
						  }
						  else if (position == (unsigned short) CheckIndexInMemory_SST(1000))
						  {
							  Usart1Send("User Unlock\r\n");
							  main_state = MAIN_TO_UNLOCK;
						  }
					  }
					  //position = (unsigned short) CheckIndexInMemory_SST(1000);
					  //sprintf(str, "user code: %03d\r\n", position);
					  //Usart1Send(str);
				  }

				  switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == RK_MUST_BE_CONTROL)
				  {
					  alarm_state = ALARM_START;	//resetea la maquina de estados de FuncAlarm()
					  main_state = MAIN_IN_ALARM;
				  }
				  else if (switches == RK_NUMBER_FINISH)
				  {
					  if (position == 951)
					  {
						  Usart1Send("Master Remote Unlock\r\n");
						  main_state = MAIN_TO_UNLOCK;
						  unlock_by_remote = 1;
					  }
					  else
					  {
						  if ((unsigned short) CheckIndexInMemory_SST(1000) == 0xFFFF)
						  {
							  if (position == 416)
							  {
								  Usart1Send("User default Remote Unlock\r\n");
								  main_state = MAIN_TO_UNLOCK;
								  unlock_by_remote = 1;
							  }
						  }
						  else if (position == (unsigned short) CheckIndexInMemory_SST(1000))
						  {
							  Usart1Send("User Remote Unlock\r\n");
							  main_state = MAIN_TO_UNLOCK;
							  unlock_by_remote = 1;
						  }
					  }
				  }
				  break;

			  case MAIN_TO_UNLOCK:
				  timer_keypad_enabled = 60000;
				  //si se esta mostrando algo espero
				  if (numbers[0] == '\0')
				  {
					  ShowNumbers(DISPLAY_REMOTE);
					  digit_remote = 1;
				  }
				  else
					  digit_remote = 0;

				  main_state = MAIN_UNLOCK;
				  break;

			  case MAIN_UNLOCK:

				  switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == KNUMBER_FINISH)
				  {
					  if (position == 800)
					  {
						  main_state = MAIN_TO_CHANGE_USER_PASSWORD;
						  Usart1Send("Change User Password\r\n");
						  wait_for_code_timeout = param_struct.wait_for_code;
					  }
					  else
					  {
						  main_state = MAIN_TO_SAVE_AT_LAST;
						  wait_for_code_timeout = param_struct.wait_for_code;
						  sprintf(str, "Guardar en: %03d\r\n", position);
						  Usart1Send(str);
					  }
				  }

				  if (!digit_remote)
				  {
					  if (numbers[0] == '\0')
					  {
						  ShowNumbers(DISPLAY_REMOTE);
						  digit_remote = 1;
					  }
				  }


				  switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == RK_MUST_BE_CONTROL)
				  {
					  alarm_state = ALARM_START;	//resetea la maquina de estados de FuncAlarm()
					  main_state = MAIN_IN_ALARM;
				  }
				  else if (switches == RK_NUMBER_FINISH)
				  {
					  main_state = MAIN_TO_SAVE_REMOTE_AT_LAST;
					  wait_for_code_timeout = param_struct.wait_for_code;
					  sprintf(str, "Keypad Remoto guardar en: %03d\r\n", position);
					  Usart1Send(str);
				  }

#ifdef CON_BLOQUEO_DE_KEYPAD
				  if (!timer_keypad_enabled)
				  {
					  Usart1Send("Keypad Locked\r\n");
					  main_state = MAIN_MAIN;
					  unlock_by_remote = 0;
					  ShowNumbers(DISPLAY_NONE);
				  }
#endif
				  break;

			  case MAIN_TO_CHANGE_USER_PASSWORD:
				  switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == KNUMBER_FINISH)
				  {
					  WriteCodeToMemory_SST(1000, position);
					  Usart1Send("User Password changed\r\n");
					  BuzzerCommands(BUZZER_SHORT_CMD, 7);
					  main_state = MAIN_TO_MAIN_WAIT_5SEGS;
				  }

				  if (switches == KCANCEL)
				  {
					  main_state = MAIN_TO_MAIN_CANCEL;
				  }

				  if (!wait_for_code_timeout)
					  main_state = MAIN_TO_MAIN_TIMEOUT;

				  break;

			  case MAIN_IN_ALARM:
				  result = FuncAlarm();
				  //if ((result == ENDED_OK) || (result == NO_CODE))
				  if (result == END_OK)
				  {
					  main_state = MAIN_TO_MAIN_WAIT_5SEGS;
				  }
				  break;

			  case MAIN_TO_SAVE_AT_LAST:

				  //me quedo esperando un código de control valido para guardarlo en posi
				  switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == RK_MUST_BE_CONTROL)
				  {
					  code = code0;
					  code <<= 16;
					  code |= code1;
					  code_position = CheckCodeInMemory(code);
					  if (code_position == 0xFFFF)
					  {
						  //el codigo no se habia utilizado
						  if (Write_Code_To_Memory(position, code) != 0)
							  Usart1Send((char *) "Codigo Guardado OK\r\n");
						  else
							  Usart1Send((char *) "Error al guardar\r\n");

						  ConvertPositionToDisplay(position);
						  BuzzerCommands(BUZZER_SHORT_CMD, 7);
					  }
					  else
					  {
						  //se habia utilizado en otra posicion
						  sprintf(str, "Error codigo ya esta en: %03d\r\n", code_position);
						  Usart1Send(str);

						  ConvertPositionToDisplay(code_position);
						  BuzzerCommands(BUZZER_HALF_CMD, 2);
					  }
					  main_state = MAIN_TO_MAIN_WAIT_5SEGS;
				  }

				  //Como tengo que conocer los tres juntos
				  mass_erase_position = position;
				  switches = CheckKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == KCANCEL)
				  {
					  main_state = MAIN_TO_MAIN_CANCEL;
				  }

				  if ((switches == KNUMBER_FINISH) && (position == 0))	//termino el numero y era 0
				  {
					  //se va a borrar la posicion con teclado local
					  position = mass_erase_position;	//update de posicion del control
					  main_state = MAIN_TO_DEL_CODE;
				  }

				  if ((switches == KNUMBER_FINISH) && (position == 999) && (mass_erase_position == 0))	//termino el numero y era 999
				  {
					  //se va a hacer un blanqueo completo
					  Usart1Send((char *) "\r\n- CUIDADO entrando en Blanqueo Completo -\r\n");
					  main_state = MAIN_TO_MASS_ERASE_AT_LAST;
				  }

				  if (!wait_for_code_timeout)
					  main_state = MAIN_TO_MAIN_TIMEOUT;

				  break;

			  case MAIN_TO_SAVE_REMOTE_AT_LAST:

				  mass_erase_position = position;
				  switches = CheckRemoteKeypad(&switches_posi0, &switches_posi1, &switches_posi2, &position);
				  if (switches == RK_CANCEL)
				  {
					  main_state = MAIN_TO_MAIN_CANCEL;
				  }

				  if ((switches == RK_NUMBER_FINISH) && (position == 0))	//termino el numero y era 0
				  {
					  //se va a borrar la posicion con teclado remoto
					  position = mass_erase_position;	//update de posicion del control
					  main_state = MAIN_TO_DEL_REMOTE_CODE;
				  }

				  if (switches == RK_MUST_BE_CONTROL)
				  {
					  code = code0;
					  code <<= 16;
					  code |= code1;
					  code_position = CheckCodeInMemory(code);
					  if (code_position == 0xFFFF)
					  {
						  //el codigo no se habia utilizado
						  if (Write_Code_To_Memory(position, code) != 0)
								Usart1Send((char *) "Codigo Guardado OK\r\n");
						  else
								Usart1Send((char *) "Error al guardar\r\n");

						  ConvertPositionToDisplay(position);
						  BuzzerCommands(BUZZER_SHORT_CMD, 7);
						  SirenCommands(SIREN_CONFIRM_OK_CMD);
					  }
					  else
					  {
						  //se habia utilizado en otra posicion
						  sprintf(str, "Error codigo ya esta en: %03d\r\n", code_position);
						  Usart1Send(str);

						  ConvertPositionToDisplay(code_position);
						  BuzzerCommands(BUZZER_HALF_CMD, 2);
						  SirenCommands(SIREN_HALF_CMD);
					  }
					  main_state = MAIN_TO_MAIN_WAIT_5SEGS;
				  }
				  if (!wait_for_code_timeout)
					  main_state = MAIN_TO_MAIN_TIMEOUT;

				  break;

			  case MAIN_TO_DEL_CODE:
				  if (Write_Code_To_Memory(position, 0xFFFFFFFF) != 0)
				  {
					  Usart1Send((char *) "Codigo Borrado OK\r\n");
					  VectorToDisplay(DISPLAY_ZERO);
					  VectorToDisplay(DISPLAY_POINT);	//agrego punto
					  VectorToDisplay(DISPLAY_NONE);
					  BuzzerCommands(BUZZER_SHORT_CMD, 7);
				  }
				  else
				  {
					  Usart1Send((char *) "Error al borrar\r\n");
					  ShowNumbers(DISPLAY_NONE);
					  BuzzerCommands(BUZZER_HALF_CMD, 1);
				  }

				  main_state = MAIN_TO_MAIN_OK;

				  /*
				  if (prog_remote)
				  {
					  if (RxCode() == ENDED_OK)
					  {
						  //en code0 y code1 tengo lo recibido
						  button_remote = CheckButtonRemote(code0, code1);

						  //se cancelo la operacion
						  if (button_remote == REM_B10)
						  {
							  ShowNumbers(DISPLAY_NONE);
							  SirenCommands(SIREN_HALF_CMD);
							  BuzzerCommands(BUZZER_HALF_CMD, 1);
							  main_state = MAIN_TO_MAIN_CANCEL;
						  }

						  //es la confirmacion
						  if (button_remote == REM_B12)
						  {
							  if (Write_Code_To_Memory(position, 0xFFFFFFFF) != 0)
							  {
								  Usart1Send((char *) "Codigo Borrado OK\r\n");
								  VectorToDisplay(DISPLAY_ZERO);
								  VectorToDisplay(DISPLAY_POINT);	//agrego punto
								  VectorToDisplay(DISPLAY_NONE);
								  SirenCommands(SIREN_CONFIRM_OK_CMD);
								  BuzzerCommands(BUZZER_SHORT_CMD, 7);
							  }
							  else
							  {
								  Usart1Send((char *) "Error al borrar\r\n");
								  ShowNumbers(DISPLAY_NONE);
								  SirenCommands(SIREN_HALF_CMD);
								  BuzzerCommands(BUZZER_HALF_CMD, 1);
							  }
							  main_state = MAIN_TO_MAIN_OK;
						  }
					  }

				  }
				    */
				  break;

			  case MAIN_TO_DEL_REMOTE_CODE:
				  if (Write_Code_To_Memory(position, 0xFFFFFFFF) != 0)
				  {
					  Usart1Send((char *) "Codigo Borrado OK\r\n");
					  VectorToDisplay(DISPLAY_ZERO);
					  VectorToDisplay(DISPLAY_POINT);	//agrego punto
					  VectorToDisplay(DISPLAY_NONE);
					  SirenCommands(SIREN_CONFIRM_OK_CMD);
					  BuzzerCommands(BUZZER_SHORT_CMD, 7);
				  }
				  else
				  {
					  Usart1Send((char *) "Error al borrar\r\n");
					  ShowNumbers(DISPLAY_NONE);
					  SirenCommands(SIREN_HALF_CMD);
					  BuzzerCommands(BUZZER_HALF_CMD, 1);
				  }

				  main_state = MAIN_TO_MAIN_OK;
				  break;

			  case MAIN_TO_MASS_ERASE_AT_LAST:
				  //se va a borrar la memoria
				  if (EraseAllMemory() != 0)
				  {
					  Usart1Send((char *) "Memoria Completa Borrada OK\r\n");
					  VectorToDisplay(DISPLAY_ZERO);
					  VectorToDisplay(DISPLAY_POINT);	//agrego punto
					  VectorToDisplay(DISPLAY_NONE);
					  BuzzerCommands(BUZZER_SHORT_CMD, 7);
					  main_state = MAIN_TO_MAIN_WAIT_5SEGS;
				  }
				  else
				  {
					  Usart1Send((char *) "Error al borrar memoria\r\n");
					  ShowNumbers(DISPLAY_NONE);
					  BuzzerCommands(BUZZER_HALF_CMD, 1);
					  main_state = MAIN_TO_MAIN_CANCEL;
				  }
				  break;

			  case MAIN_TO_MAIN_CANCEL:
				  Usart1Send((char *) "Opereta cancelada\r\n");
				  main_state = MAIN_INIT;
				  interdigit_timeout = 300;	//espero que se limpien las teclas
				  break;

			  case MAIN_TO_MAIN_TIMEOUT:
				  Usart1Send((char *) "Timeout\r\n");
				  main_state = MAIN_INIT;
				  interdigit_timeout = 300;	//espero que se limpien las teclas
				  break;

			  case MAIN_TO_MONITORING:
				  Usart1Send((char *) "Goto 115200 confirmed\r\n");
				  main_state++;
				  interdigit_timeout = 300;	//espero que se limpien los buffers
				  break;

			  case MAIN_TO_MONITORINGA:
				  if (!interdigit_timeout)
				  {
					  ShowNumbers(DISPLAY_REMOTE);
					  USART1->CR1 &= ~USART_CR1_UE;
					  USART1->BRR = USART_115200;
					  USART1->CR1 |= USART_CR1_UE;
					  main_state++;
				  }
				  break;

			  case MAIN_TO_MONITORINGB:
				  Usart1Send((char *) "Monitoring confirmed\r\n");	//enviado a 115200
				  main_state++;
				  break;

			  case MAIN_TO_MONITORINGC:

				  break;


			  case MAIN_TO_MONITORING_LEAVE:
				  Usart1Send((char *) "Goto 9600 confirmed\r\n");
				  main_state++;
				  interdigit_timeout = 300;	//espero que se limpien los buffers
				  break;

			  case MAIN_TO_MONITORINGE:
				  if (!interdigit_timeout)
				  {
					  ShowNumbers(DISPLAY_NONE);
					  USART1->CR1 &= ~USART_CR1_UE;
					  USART1->BRR = USART_9600;
					  USART1->CR1 |= USART_CR1_UE;
					  main_state = MAIN_TO_MAIN_OK;
				  }
				  break;

			  case MAIN_TO_MAIN_WAIT_5SEGS:
				  interdigit_timeout = ACT_DESACT_IN_MSECS;	//espero 5 segundos luego del codigo grabado OK
				  main_state = MAIN_TO_MAIN_WAIT_5SEGSA;
				  break;

			  case MAIN_TO_MAIN_WAIT_5SEGSA:
				  if (!interdigit_timeout)
					  main_state = MAIN_TO_MAIN_OK;
				  break;

			  case MAIN_TO_MAIN_OK:
				  main_state = MAIN_INIT;
				  break;

			  case MAIN_MEMORY_DUMP:
					//cargo 256 numeros al vector de 1024 posiciones
				  MemoryDump (OFFSET_CODES_256);
				  main_state = MAIN_INIT;
				  break;

			  case MAIN_MEMORY_DUMP2:
					//cargo 256 numeros al vector de 1024 posiciones
				  MemoryDump (OFFSET_CODES_512);
				  main_state = MAIN_INIT;
				  break;

			  default:
				  main_state = MAIN_INIT;
				  break;

		  }
		  /*
			if (LED)
			{
				LED_OFF;
				FPLUS_OFF;
				F12PLUS_OFF;
				F5PLUS_OFF;
				//BUZZER_OFF;
				//WP_ON;
				//CE_ON;
				Power_Ampli_Ena();
			}
			else
			{
				LED_ON;
				FPLUS_ON;
				F12PLUS_ON;
				F5PLUS_ON;
				//BUZZER_ON;
				//WP_OFF;
				//CE_OFF;
				Power_Ampli_Disa();
			}
		   */
		  /*
			i = (unsigned char) ReadSwitches();
			ShowNumbers(i);
			//Wait_ms(50);
			if (buffrx_ready)
			{
				j = InterpretarMsg(pbuffrx);
				buffrx_ready = 0;
				if (j != ERROR)
					Usart1Send((const char *) "OK\r\n");
			}

			if (RxCode() == ENDED_OK)
			{
				code = code0;
				code <<= 16;
				code |= code1;

				dummy16 = CheckCodeInMemory (code);
				if (dummy16 != 0xffff)
				{
					sprintf(str, "Code Finded in: %04d\r\n", dummy16);
					Usart1Send(str);
				}
				else
				{
					sprintf(str, "Code: %04X %04X not finded\r\n", code0, code1);
					Usart1Send(str);
				}
			}
			*/

			/*
			Wait_ms(2000);
			if (ReadMem())
				Usart1Send((const char *) "Mem OK\r\n");
			else
				Usart1Send((const char *) "Mem NOK!!\r\n");
			*/

			/*
			if (j == 0)
			{
				dummy16 = Write_Code_To_Memory(512, 0x4D08064A);
				j = 1;
				if (dummy16 == 0)
					Usart1Send((const char *) "No grabo\r\n");
				else
					Usart1Send((const char *) "Grabado OK\r\n");
			}
			while(1);
			*/
		  UpdateBuzzer();
		  UpdateDisplay();
		  UpdateSiren();
		  UpdateAudio();
#ifdef CON_MODIFICACION_DIODO_BATERIA
		  UpdateBattery();
#endif
		  //OJO esta rutina puede cambiar el estado del programa principal
		  main_state = UpdateUart(main_state);
	  }

	  //--- FIN PRUEBA DE NUMEROS DEL 0 AL 9 y TACT SWITCHES

	  //--- PRUEBA DE SALIDAS A TRANSISTOR
	 /*
	  while (1)
	  {
		  FPLUS_ON;
		  F12PLUS_ON;
		  F5PLUS_ON;
		  LED_ON;
		  BUZZER_ON;

		  Wait_ms(1000);

		  FPLUS_OFF;
		  F12PLUS_OFF;
		  F5PLUS_OFF;
		  LED_OFF;
		  BUZZER_OFF;

		  Wait_ms(1000);
	  }
	*/
	  //--- FIN PRUEBA DE SALIDAS A TRANSISTOR

	  //--- PRUEBA LEER MEMORIA
	  /*
	  while (1)
	  {
			if (LED)
				LED_OFF;
			else
				LED_ON;

			if (readJEDEC())
			//if (readStatusNVM())
				Wait_ms(50);
			else
				Wait_ms(500);
	  }
	  */
	  //--- FIN PRUEBA LEER MEMORIA



	  //FIN PRUEBA DE SPI


	//--- PRUEBAS PWM
	  /*
	//Update_PWM(1024);
	 Update_PWM(666, 333, 1);
	 while (1)
	 {
			if (LED)
			{
				LED_OFF;
				Update_PWM(1000, 500, 1);
			}
			else
			{
				LED_ON;
				Update_PWM(500, 250, 1);
			}


			Wait_ms(800);

	 }
	 */
	 //--- FIN PRUEBAS PWM

	//ADC configuration.
	//if (ADC_Conf() == 0)
	//{
	//	while (1)
	//	{
	//		if (LED)
	//			LED_OFF;
	//		else
	//			LED_ON;
	//	}
	//}

	return 0;
}
//--- End of Main ---//

//funcion de alarmas, revisa codigo en memoria y actua en consecuencia
unsigned char FuncAlarm (void)
{
	unsigned char button;
	unsigned int code;
	unsigned short code_position;

	char str[50];

	switch (alarm_state)
	{
		case ALARM_START:
			alarm_state = ALARM_NO_CODE;
			code = code0;
			code <<= 16;
			code |= code1;

			//code_position = CheckCodeInMemory(code);
			code_position = CheckBaseCodeInMemory(code);

			if ((code_position >= 0) && (code_position <= 1023))
			{
				sprintf(str, (char *) "Activo: %03d ", code_position);
				//el codigo existe en memoria
				//reviso el boton
				button = CheckButtonInCode(code);
				if (button == 1)
				{
					last_one_or_three = code_position;
					alarm_state = ALARM_BUTTON1;
					strcat(str, (char *) "B1\r\n");
					repetition_counter = param_struct.b1r;
				}
				else if (button == 2)
				{
					//original boton 2
					last_two = code_position;
					alarm_state = ALARM_BUTTON2;
					strcat(str, (char *) "B2\r\n");
					//para pruebas boton 2 -> 3
					//last_one_or_three = code_position;
					//repetition_counter = b3r;
					//alarm_state = ALARM_BUTTON3;
					//strcat(str, (char *) "B3\r\n");
					//para pruebas boton 2 -> 4
					//alarm_state = ALARM_BUTTON4;
					//strcat(str, (char *) "B4\r\n");
					//repetition_counter = b4r;
				}
				else if (button == 3)
				{
					last_one_or_three = code_position;
					alarm_state = ALARM_BUTTON3;
					strcat(str, (char *) "B3\r\n");
					repetition_counter = param_struct.b3r;
				}
				else if (button == 4)
				{
					alarm_state = ALARM_BUTTON4;
					strcat(str, (char *) "B4\r\n");
					repetition_counter = param_struct.b4r;
				}

				//button_timer = 5;	//5 segundos suena seguro EVITA PROBLEMAS EN LA VUELTA
				button_timer = ACT_DESACT_IN_SECS;	//2 segundos OK y buena distancia 20-5-15


				Usart1Send(str);
			}
			break;

		case ALARM_BUTTON1:
			FPLUS_ON;
			F5PLUS_ON;
			F12PLUS_ON;
			SirenCommands(SIREN_MULTIPLE_UP_CMD);
			alarm_state++;
			break;

		case ALARM_BUTTON1_A:
			if (!button_timer)
			{
				button_timer = param_struct.b1t;
				alarm_state++;
			}
			break;

		case ALARM_BUTTON1_B:
			//me quedo esperando que apaguen o timer
#ifdef PROGRAMA_NORMAL
			if (CheckForButtons(&code_position, &code) == 1)	//reviso solo boton 1
			{
				alarm_state = ALARM_BUTTON1_FINISH;
			}
#endif

#ifdef PROGRAMA_DE_BUCLE
			if (CheckForButtons(&code_position, &code) == 4)	//desactivo solo con 4
			{
				alarm_state = ALARM_BUTTON1_FINISH;
			}
#endif

			if (!button_timer)
			{
				//tengo timeout, avanzo al audio
				alarm_state++;
			}
			break;

		case ALARM_BUTTON1_C:
			//paso el audio y descuento un ciclo
			if (repetition_counter > 1)
			{
				alarm_state++;
				repetition_counter--;
			}
			else
			{
				Usart1Send((char*) "Timeout B1 ");
				alarm_state = ALARM_BUTTON1_FINISH;
			}
			break;

		case ALARM_BUTTON1_D:
			SirenCommands(SIREN_STOP_CMD);
			PositionToSpeak(last_one_or_three);
			alarm_state++;
			break;

		case ALARM_BUTTON1_E:
			if (audio_state == AUDIO_INIT)
			{
				//termino de enviar audio
				alarm_state = ALARM_BUTTON1;
			}
			break;

		case ALARM_BUTTON1_FINISH:
			sprintf(str, "Desactivo: %03d\r\n", last_one_or_three);
			Usart1Send(str);

			SirenCommands(SIREN_STOP_CMD);
			FPLUS_OFF;
			F5PLUS_OFF;
			F12PLUS_OFF;

			PositionToSpeak(last_one_or_three);
			alarm_state++;
			break;

		case ALARM_BUTTON1_FINISH_B:
			if (audio_state == AUDIO_INIT)
			{
				//termino de enviar audio
				return END_OK;
			}
			break;

		case ALARM_BUTTON2:		//solo enciendo reflectores
			FPLUS_ON;

			alarm_state++;
#ifdef HT6P20B2_FOURTH_BUTTON
			button_timer = 4;	//4 segundos overdrive
#endif
			break;

		case ALARM_BUTTON2_A:		//espero los primeros 2 segundos
#ifdef HT6P20B2_FOURTH_BUTTON
			if (!button_timer)
			{
				button_timer = 2;
				alarm_state++;
			}
#else
			alarm_state++;
#endif
			break;

		case ALARM_BUTTON2_B:
#ifdef HT6P20B2_FOURTH_BUTTON
			if (CheckForButtons(&code_position, &code) == 2)	//reviso solo boton 2 para simular el 4
			{
				if (code_position == last_two)
					alarm_state = ALARM_BUTTON2_D;
			}
#endif
			if (!button_timer)
			{
				button_timer = param_struct.b2t;
				alarm_state++;
			}
			break;

		case ALARM_BUTTON2_C:
			//me quedo esperando que apaguen o timer

			button = CheckForButtons(&code_position, &code);
#ifdef PROGRAMA_NORMAL
			if (button == 2)	//reviso solo boton 2
			{
				sprintf(str, "Desactivo: %03d\r\n", code_position);
				Usart1Send(str);
				alarm_state = ALARM_BUTTON2_FINISH;
			}

			if (button == 1)		//reviso el boton
			{
				sprintf(str, "Activo: %03d B1\r\n", code_position);
				Usart1Send(str);

				last_one_or_three = code_position;
				repetition_counter = param_struct.b1r;
				button_timer = ACT_DESACT_IN_SECS;

				alarm_state = ALARM_BUTTON1;
			}

			if (button == 3)		//reviso el boton
			{
				sprintf(str, "Activo: %03d B3\r\n", code_position);
				Usart1Send(str);

				last_one_or_three = code_position;
				repetition_counter = param_struct.b3r;
				button_timer = ACT_DESACT_IN_SECS;

				alarm_state = ALARM_BUTTON3;
			}
#endif
#ifdef PROGRAMA_DE_BUCLE
			if (button == 1)		//reviso el boton
			{
				sprintf(str, "Activo: %03d B1\r\n", code_position);
				Usart1Send(str);

				last_one_or_three = code_position;
				repetition_counter = param_struct.b1r;
				button_timer = ACT_DESACT_IN_SECS;

				alarm_state = ALARM_BUTTON1;
			}

			if (button == 3)		//reviso el boton
			{
				sprintf(str, "Activo: %03d B3\r\n", code_position);
				Usart1Send(str);

				last_one_or_three = code_position;
				repetition_counter = param_struct.b3r;
				button_timer = ACT_DESACT_IN_SECS;

				alarm_state = ALARM_BUTTON3;
			}

			if (button == 4)	//reviso boton 4 para desactivar
			{
				sprintf(str, "Desactivo: %03d\r\n", code_position);
				Usart1Send(str);
				alarm_state = ALARM_BUTTON2_FINISH;
			}
#endif

			if (!button_timer)
			{
				//tengo timeout, apago reflectores
				Usart1Send((char*) "Timeout B2\r\n");
				alarm_state = ALARM_BUTTON2_FINISH;
			}
			break;

		case ALARM_BUTTON2_D:
			SirenCommands(SIREN_STOP_CMD);
			PositionToSpeak(last_one_or_three);
			alarm_state++;
			break;

		case ALARM_BUTTON2_E:
			if (audio_state == AUDIO_INIT)
			{
				//termino de enviar audio
				alarm_state++;
			}
			break;

		case ALARM_BUTTON2_FINISH:
			FPLUS_OFF;
			return END_OK;
			break;

		case ALARM_BUTTON3:
			FPLUS_ON;
//			F5PLUS_ON;
//			F12PLUS_ON;
			//SirenCommands(SIREN_SINGLE_CMD);
			SirenCommands(SIREN_SINGLE_CHOPP_CMD);
			//button_timer = 5;	//5 segundos suena seguro EVITA PROBLEMAS EN LA VUELTA
			alarm_state++;
			break;

		case ALARM_BUTTON3_A:
			if (!button_timer)
			{
				button_timer = param_struct.b3t;
				alarm_state++;
			}
			break;

		case ALARM_BUTTON3_B:
			//me quedo esperando que apaguen o timer

			button = CheckForButtons(&code_position, &code);
#ifdef PROGRAMA_NORMAL
			if (button == 3)	//reviso solo boton 3
			{
				sprintf(str, "Desactivo: %03d\r\n", code_position);
				Usart1Send(str);
				alarm_state = ALARM_BUTTON3_FINISH;
			}

			if (button == 1)		//reviso el boton
			{
				sprintf(str, "Activo: %03d B1\r\n", code_position);
				Usart1Send(str);

				last_one_or_three = code_position;
				repetition_counter = param_struct.b1r;
				button_timer = ACT_DESACT_IN_SECS;

				alarm_state = ALARM_BUTTON1;
			}
#endif

#ifdef PROGRAMA_DE_BUCLE
			if (button == 1)		//reviso el boton
			{
				sprintf(str, "Activo: %03d B1\r\n", code_position);
				Usart1Send(str);

				last_one_or_three = code_position;
				repetition_counter = param_struct.b1r;
				button_timer = ACT_DESACT_IN_SECS;

				alarm_state = ALARM_BUTTON1;
			}

			if (button == 4)	//reviso solo boton 4
			{
				sprintf(str, "Desactivo: %03d\r\n", code_position);
				Usart1Send(str);
				alarm_state = ALARM_BUTTON3_FINISH;
			}
#endif

			if (!button_timer)
			{
				//tengo timeout, avanzo al audio

				alarm_state++;
			}
			break;

		case ALARM_BUTTON3_C:
			//paso el audio y descuento un ciclo
			if (repetition_counter > 1)
			{
				alarm_state++;
				repetition_counter--;
			}
			else
			{
				Usart1Send((char*) "Timeout B3 ");
				alarm_state = ALARM_BUTTON3_FINISH;
			}
			break;

		case ALARM_BUTTON3_D:
			SirenCommands(SIREN_STOP_CMD);
			PositionToSpeak(last_one_or_three);
			alarm_state++;
			break;

		case ALARM_BUTTON3_E:
			if (audio_state == AUDIO_INIT)
			{
				//termino de enviar audio
				alarm_state = ALARM_BUTTON3;
			}
			break;

		case ALARM_BUTTON3_FINISH:
			sprintf(str, "Desactivo: %03d\r\n", last_one_or_three);
			Usart1Send(str);

			SirenCommands(SIREN_STOP_CMD);
//			F12PLUS_OFF;
			FPLUS_OFF;

			PositionToSpeak(last_one_or_three);
			alarm_state++;
			break;

		case ALARM_BUTTON3_FINISH_B:
			if (audio_state == AUDIO_INIT)
			{
				//termino de enviar audio
				return END_OK;
			}
			break;

		case ALARM_BUTTON4:
			SirenCommands(SIREN_STOP_CMD);
			PositionToSpeak(last_one_or_three);
			alarm_state++;
			break;

		case ALARM_BUTTON4_A:
			if (audio_state == AUDIO_INIT)
			{
				//termino de enviar audio
				alarm_state++;
			}
			break;

		case ALARM_BUTTON4_FINISH:
			return END_OK;
			break;

		case ALARM_NO_CODE:
			return END_OK;
			break;
		default:
			alarm_state = 0;
			break;
	}
	/*
		if (sirena_sonando)
		{
			sirena_sonando = 0;
			SirenCommands(SIREN_STOP_CMD);
			sprintf(str, "Desactivo: %03d\r\n", code_position);
			Usart1Send(str);
		}
		else
		{
			sirena_sonando = 1;
			SirenCommands(SIREN_MULTIPLE_UP_CMD);
			sprintf(str, "Activo: %03d\r\n", code_position);
			Usart1Send(str);
		}
		main_state = MAIN_TO_MAIN_WAIT_5SEGS;

	}
	*/

	return WORKING;
}


/*
void SendSegment (unsigned char segment)
{
	unsigned char j;

	OE_OFF;
    while (SPI_GetTransmissionFIFOStatus(SPIx) != SPI_TransmissionFIFOStatus_Empty);
    if (segment & 0x80)
    	SPI_SendData8(SPIx, 0x01);
    else
    	SPI_SendData8(SPIx, 0x00);

    while (SPI_GetTransmissionFIFOStatus(SPIx) != SPI_TransmissionFIFOStatus_Empty);
    segment <<= 1;
    SPI_SendData8(SPIx, segment);
    for (j = 0; j < 150; j++)
    {
    	asm("nop");
    }
	OE_ON;
}
*/

void SendSegment (unsigned char segment)
{
	OE_OFF;
    if (segment & 0x80)
    	Send_SPI_Single (0x01);
    else
    	Send_SPI_Single (0x00);

    segment <<= 1;
    Send_SPI_Single (segment);
	OE_ON;
}

void ShowNumbersAgain (void)
{
	ShowNumbers (last_digit);
}

//		dp g f e d c b a
//bits   7 6 5 4 3 2 1 0
//negados
void ShowNumbers (unsigned char number)	//del 1 al 9; 10 es cero; 11 es punto; 0, 12, 13, 14, 15 apagar
{
	//unsigned char a;
	//me fijo si lo saco con punto
	//if (number > 128)

	//number = number & 0x0F;
	//switch (number)
	last_digit = number;
	switch (number)
	{
		case 10:
			//number = 0x3F;
			number = 0xC0;
			break;

		case DISPLAY_ERROR:
			//E
			number = 0x06;
			break;

		case DISPLAY_ERROR2:
			//E inv
			number = 0xB0;
			break;

		case DISPLAY_LINE:
			//raya media
			number = 0xBF;
			break;

		case DISPLAY_REMOTE:
			//cuadro superior
			number = 0x9C;
			break;

		case DISPLAY_SQR_DOWN:
			//cuadro inferior
			number = 0xA3;
			break;

		case 1:
			//number = 0x06;
			number = 0xF9;
			break;
		case 2:
			//number = 0x5B;
			number = 0xA4;
			break;
		case 3:
			//number = 0x4F;
			number = 0xB0;
			break;
		case 4:
			//number = 0x66;
			number = 0x99;
			break;
		case 5:
			//number = 0x6D;
			number = 0x92;
			break;
		case 6:
			//number = 0x7D;
			number = 0x82;
			break;
		case 7:
			//number = 0x07;
			number = 0xF8;
			break;
		case 8:
			//number = 0x7F;
			number = 0x80;
			break;
		case 9:
			//number = 0x67;
			number = 0x98;
			break;
		case 11:
			//number = 0x80;	//solo punto
			number = 0x7F;	//solo punto
			break;

		default:
			//apagar el display
			number = 0xFF;
			break;
	}
	SendSegment (number);
}

unsigned char ReadSwitches (void)
{
	//funciona tipo Zero Order Hold con tiempo de muestreo SWITCHES_INTERNALS_TIMEOUT
	//revisa los switches cada 50ms, le da tiempo al display para mostrar los numeros
	if (!switches_internals_timeout)
	{
		last_internals = ReadSwitches_Internals ();
		ShowNumbersAgain();
		switches_internals_timeout = SWITCHES_INTERNALS_TIMEOUT;
	}
	return last_internals;
}

#define SPIx                             SPI1
unsigned char ReadSwitches_Internals (void)
{
	unsigned short sw = 0;
	unsigned char sw0 = 0;
	unsigned char sw1 = 0;

	//bajo la velocidad del SPI
	SPIx->CR1 &= ~SPI_CR1_SPE;		//deshabilito periferico
	//clk / 256;
	SPIx->CR1 &= 0xFFC7;
	SPIx->CR1 |= SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2;

	SPIx->CR1 |= SPI_CR1_SPE;		//habilito periferico

	OE_OFF;
	PS_ON;

    sw0 = Receive_SPI_Single();
    sw1 = Receive_SPI_Single();

	PS_OFF;
	OE_ON;

	sw = sw1;
	sw <<= 8;
	sw |= sw0;

	switch (sw)
	{
		case 0xBFBE:
			sw = 1;
			break;

		case 0xBFBD:
			sw = 2;
			break;

		case 0xBFBB:
			sw = 3;
			break;

		case 0xBFB7:
			sw = 4;
			break;

		case 0xBFAF:
			sw = 5;
			break;

		case 0xBF9F:
			sw = 6;
			break;

		case 0xBEBF:
			sw = 7;
			break;

		case 0xBDBF:
			sw = 8;
			break;

		case 0xBBBF:
			sw = 9;
			break;

		case 0xB7BF:
			sw = 10;
			break;

		case 0xAFBF:
			sw = 11;
			break;

		case 0x9FBF:
			sw = 12;
			break;

		default:
			sw = 0;
			break;
	}

	//subo la velocidad del SPI
	SPIx->CR1 &= ~SPI_CR1_SPE;		//deshabilito periferico
	//clk / 8;
	SPIx->CR1 &= 0xFFC7;
	SPIx->CR1 |= SPI_CR1_BR_1;

	SPIx->CR1 |= SPI_CR1_SPE;		//habilito periferico

	return (unsigned char) sw;
}

unsigned char CheckKeypad (unsigned char * sp0, unsigned char * sp1, unsigned char * sp2, unsigned short * posi)
{
	unsigned char switches = 0;

	switches = ReadSwitches();

	switch (keypad_state)
	{
		case KNONE:

			if ((switches != NO_KEY) && (switches != STAR_KEY) && (switches != POUND_KEY))
			{
				//se presiono un numero voy a modo grabar codigo
				//reviso si fue 0
				if (switches == ZERO_KEY)
				{
					*sp0 = 0;
					ShowNumbers(10);
				}
				else
				{
					*sp0 = switches;
					ShowNumbers(switches);
				}
				BuzzerCommands(BUZZER_SHORT_CMD, 1);
				*sp1 = 0;
				*sp2 = 0;
				interdigit_timeout = param_struct.interdigit;
				keypad_state = KRECEIVING_A;
			}

			if (switches == STAR_KEY)
			{
				//se cancelo la operacion
				ShowNumbers(DISPLAY_NONE);
				BuzzerCommands(BUZZER_HALF_CMD, 1);
				keypad_state = KCANCEL;
			}
			break;

		case KRECEIVING_A:
			//para validar switch anterior necesito que lo liberen
			if (switches == 0)
			{
				ShowNumbers(DISPLAY_NONE);
				keypad_state = KRECEIVING_B;
				interdigit_timeout = param_struct.interdigit;
			}

			if (!interdigit_timeout)
				keypad_state = KTIMEOUT;

			break;

		case KRECEIVING_B:			//segundo digito o confirmacion del primero
			if (switches == STAR_KEY)
			{
				//se cancelo la operacion
				ShowNumbers(DISPLAY_NONE);
				BuzzerCommands(BUZZER_HALF_CMD, 1);
				keypad_state = KCANCEL;
			}

			if (((switches > NO_KEY) && (switches < 10)) || (switches == ZERO_KEY))	//es un numero 1 a 9 o 0
			{
				if (switches == ZERO_KEY)
				{
					*sp1 = 0;
					ShowNumbers(10);
				}
				else
				{
					*sp1 = switches;
					ShowNumbers(switches);
				}
				BuzzerCommands(BUZZER_SHORT_CMD, 1);
				keypad_state = KRECEIVING_C;
				interdigit_timeout = param_struct.interdigit;
			}

			//si esta apurado un solo numero
			if (switches == POUND_KEY)
			{
				ShowNumbers(DISPLAY_LINE);
				BuzzerCommands(BUZZER_SHORT_CMD, 2);
				*sp2 = *sp0;
				*sp0 = 0;
				*posi = *sp2;

				keypad_state = KNUMBER_FINISH;
			}

			if (!interdigit_timeout)
				  keypad_state = KTIMEOUT;

			break;

		case KRECEIVING_C:
			  //para validar switch anterior necesito que lo liberen
			  if (switches == 0)
			  {
				  ShowNumbers(DISPLAY_NONE);
				  keypad_state = KRECEIVING_D;
				  interdigit_timeout = param_struct.interdigit;
			  }

			  if (!interdigit_timeout)
				  keypad_state = KTIMEOUT;

			break;

		case KRECEIVING_D:				//tercer digito o confirmacion del segundo
			  if (switches == STAR_KEY)
			  {
				  //se cancelo la operacion
				  ShowNumbers(DISPLAY_NONE);
				  BuzzerCommands(BUZZER_HALF_CMD, 1);
				  keypad_state = KCANCEL;
			  }

			  if (((switches > NO_KEY) && (switches < 10)) || (switches == ZERO_KEY))	//es un numero 1 a 9 o 0
			  {
				  if (switches == ZERO_KEY)
				  {
					  *sp2 = 0;
					  ShowNumbers(10);
				  }
				  else
				  {
					  *sp2 = switches;
					  ShowNumbers(switches);
				  }

				  BuzzerCommands(BUZZER_SHORT_CMD, 1);
				  keypad_state = KRECEIVING_E;
				  interdigit_timeout = param_struct.interdigit;
			  }

			  //si esta apurado dos numeros
			  if (switches == POUND_KEY)
			  {
				  ShowNumbers(DISPLAY_LINE);
				  BuzzerCommands(BUZZER_SHORT_CMD, 2);
				  *sp2 = *sp0;
				  *posi = *sp2 * 10 + *sp1;

				  keypad_state = KNUMBER_FINISH;
			  }

			  if (!interdigit_timeout)
				  keypad_state = KTIMEOUT;

			break;

		case KRECEIVING_E:
			  //para validar switch anterior necesito que lo liberen
			  if (switches == 0)
			  {
				  ShowNumbers(DISPLAY_NONE);
				  keypad_state = KRECEIVING_F;
				  interdigit_timeout = param_struct.interdigit;

			  }

			  if (!interdigit_timeout)
				  keypad_state = KTIMEOUT;

			break;

		case KRECEIVING_F:
			if (switches == STAR_KEY)
			{
				//se cancelo la operacion
				ShowNumbers(DISPLAY_NONE);
				BuzzerCommands(BUZZER_HALF_CMD, 1);
				keypad_state = KCANCEL;
			}

			if (switches == POUND_KEY)	//es la confirmacion
			{
				ShowNumbers(DISPLAY_LINE);
				BuzzerCommands(BUZZER_SHORT_CMD, 2);
				*posi = *sp0 * 100 + *sp1 * 10 + *sp2;

				keypad_state = KNUMBER_FINISH;
			}

			if (!interdigit_timeout)
				keypad_state = KTIMEOUT;
			break;

		case KNUMBER_FINISH:
		case KCANCEL:
		case KTIMEOUT:
		default:
			keypad_state = KNONE;
			break;
	}

	return keypad_state;
}

unsigned char CheckRemoteKeypad (unsigned char * sp0, unsigned char * sp1, unsigned char * sp2, unsigned short * posi)
{
	unsigned char button_remote = 0;

	switch (remote_keypad_state)
	{
		case RK_NONE:
			//me quedo esperando un código de control
			if (RxCode() == ENDED_OK)
			{
				//reviso aca si es de remote keypad o control
				//si es control contesto MUST_BE_CONTROL

				//en code0 y code1 tengo lo recibido
				button_remote = CheckButtonRemote(code0, code1);

				if (button_remote != REM_NO)
				{
					//se cancelo la operacion
					if (button_remote == REM_B10)
					{
						ShowNumbers(DISPLAY_NONE);
						if (unlock_by_remote)
							SirenCommands(SIREN_HALF_CMD);
						BuzzerCommands(BUZZER_HALF_CMD, 1);
						remote_keypad_state = RK_CANCEL;
					}
					else if (((button_remote > REM_NO) && (button_remote < REM_B10)) || (button_remote == REM_B11))	//es un numero 1 a 9 o 0
					{
						//se presiono un numero - reviso si fue 0
						if (button_remote == REM_B11)
						{
							ShowNumbers(10);
							*sp0 = 0;
						}
						else
						{
							ShowNumbers(button_remote);
							*sp0 = button_remote;
						}
						if (unlock_by_remote)
							SirenCommands(SIREN_SHORT_CMD);
						BuzzerCommands(BUZZER_SHORT_CMD, 1);

						*sp1 = 0;
						*sp2 = 0;
						remote_keypad_state = RK_RECEIVING_A;
						interdigit_timeout = 1000;
					}
				}
				else
					remote_keypad_state = RK_MUST_BE_CONTROL;
			}
			break;

		case RK_RECEIVING_A:
			if (!interdigit_timeout)
			{
				remote_keypad_state = RK_RECEIVING_B;
				interdigit_timeout = param_struct.interdigit;
			}
			break;

		case RK_RECEIVING_B:			//segundo digito o confirmacion del primero
			if (RxCode() == ENDED_OK)
			{
				//en code0 y code1 tengo lo recibido
				button_remote = CheckButtonRemote(code0, code1);

				//se cancelo la operacion
				if (button_remote == REM_B10)
				{
					ShowNumbers(DISPLAY_NONE);
					if (unlock_by_remote)
						SirenCommands(SIREN_HALF_CMD);
					BuzzerCommands(BUZZER_HALF_CMD, 1);
					remote_keypad_state = RK_CANCEL;
				}

				//si esta apurado un solo numero
				if (button_remote == REM_B12)
				{
					ShowNumbers(DISPLAY_LINE);
					if (unlock_by_remote)
						SirenCommands(SIREN_SHORT_CMD);
					BuzzerCommands(BUZZER_SHORT_CMD, 2);
					*sp2 = *sp0;
					*sp1 = 0;
					*sp0 = 0;

					*posi = *sp2;
					remote_keypad_state = RK_NUMBER_FINISH;
				}
				//es un numero 1 a 9 o 0
				if (((button_remote > REM_NO) && (button_remote < REM_B10)) || (button_remote == REM_B11))
				{
					if (button_remote == REM_B11)
					{
						*sp1 = 0;
						ShowNumbers(10);
					}
					else
					{
						*sp1 = button_remote;
						ShowNumbers(button_remote);
					}

					if (unlock_by_remote)
						SirenCommands(SIREN_SHORT_CMD);
					BuzzerCommands(BUZZER_SHORT_CMD, 1);
					*sp2 = 0;
					remote_keypad_state = RK_RECEIVING_C;
					interdigit_timeout = 1000;
				}
			}

			if (!interdigit_timeout)
			{
				remote_keypad_state = RK_TIMEOUT;
			}

			break;

		case RK_RECEIVING_C:
			if (!interdigit_timeout)
			{
				remote_keypad_state = RK_RECEIVING_D;
				interdigit_timeout = param_struct.interdigit;
			}
			break;

		case RK_RECEIVING_D:				//tercer digito o confirmacion del segundo
			if (RxCode() == ENDED_OK)
			{
				//en code0 y code1 tengo lo recibido
				button_remote = CheckButtonRemote(code0, code1);

				//se cancelo la operacion
				if (button_remote == REM_B10)
				{
					ShowNumbers(DISPLAY_NONE);
					if (unlock_by_remote)
						SirenCommands(SIREN_HALF_CMD);
					BuzzerCommands(BUZZER_HALF_CMD, 1);
					remote_keypad_state = RK_CANCEL;
				}

				//si esta apurado dos numeros
				if (button_remote == REM_B12)
				{
					ShowNumbers(DISPLAY_LINE);
					if (unlock_by_remote)
						SirenCommands(SIREN_SHORT_CMD);
					BuzzerCommands(BUZZER_SHORT_CMD, 2);
					*sp2 = *sp0;
					*sp0 = 0;

					*posi = *sp2 * 10 + *sp1;
					remote_keypad_state = RK_NUMBER_FINISH;
				}

				if (((button_remote > REM_NO) && (button_remote < REM_B10)) || (button_remote == REM_B11))	//es un numero 1 a 9 o 0
				{
					if (button_remote == ZERO_KEY)
					{
						*sp2 = 0;
						ShowNumbers(10);
					}
					else
					{
						*sp2 = button_remote;
						ShowNumbers(button_remote);
					}
					if (unlock_by_remote)
						SirenCommands(SIREN_SHORT_CMD);
					BuzzerCommands(BUZZER_SHORT_CMD, 1);
					remote_keypad_state = RK_RECEIVING_E;
					interdigit_timeout = 1000;
				}
			}

			if (!interdigit_timeout)
			{
				remote_keypad_state = RK_TIMEOUT;
			}
			break;

		case RK_RECEIVING_E:
			if (!interdigit_timeout)
			{
				remote_keypad_state = RK_RECEIVING_F;
				interdigit_timeout = param_struct.interdigit;
			}
			break;

		case RK_RECEIVING_F:
			if (RxCode() == ENDED_OK)
			{
				//en code0 y code1 tengo lo recibido
				button_remote = CheckButtonRemote(code0, code1);

				//se cancelo la operacion
				if (button_remote == REM_B10)
				{
					ShowNumbers(DISPLAY_NONE);
					if (unlock_by_remote)
						SirenCommands(SIREN_HALF_CMD);
					BuzzerCommands(BUZZER_HALF_CMD, 1);
					remote_keypad_state = RK_CANCEL;
				}

				//es la confirmacion
				if (button_remote == REM_B12)
				{
					ShowNumbers(DISPLAY_LINE);
					if (unlock_by_remote)
						SirenCommands(SIREN_SHORT_CMD);
					BuzzerCommands(BUZZER_SHORT_CMD, 2);
					*posi = *sp0 * 100 + *sp1 * 10 + *sp2;
					remote_keypad_state = RK_NUMBER_FINISH;
				}
			}

			if (!interdigit_timeout)
			{
				remote_keypad_state = RK_TIMEOUT;
			}
			break;

		case RK_MUST_BE_CONTROL:
		case RK_CANCEL:
		case RK_TIMEOUT:
		default:
			remote_keypad_state = RK_NONE;
			break;
	}
	return remote_keypad_state;
}

//carga los numeros a mostrar en secuencia en un vector
//del 1 al 9; 10 es cero; 11 es punto; 0, 12, 13, 14, 15 apagar
void VectorToDisplay (unsigned char new_number)
{
	unsigned char i;
	//me fijo si hay espacio
	if (p_numbers < &numbers[LAST_NUMBER])
	{
		//busco la primer posicion vacia y pongo el nuevo numero
		for (i = 0; i < LAST_NUMBER; i++)
		{
			if (numbers[i] == 0)
			{
				numbers[i] = new_number;
				i = LAST_NUMBER;
			}
		}
	}
}

//caraga la posicion con VectorToSpeak
void PositionToSpeak(unsigned short posi)
{
	unsigned char a, b, c;

	a = posi / 100;

	if (a == 0)
		VectorToSpeak(10);
	else
		VectorToSpeak(a);

	b = (posi - a * 100) / 10;

	if (b == 0)
		VectorToSpeak(10);
	else
		VectorToSpeak(b);

	c = (posi - a * 100 - b * 10);

	if (c == 0)
		VectorToSpeak(10);
	else
		VectorToSpeak(c);

	VectorToSpeak(0);
}

//carga los numeros a decir en secuencia en un vector
//del 1 al 9; 10 es cero
void VectorToSpeak (unsigned char new_number)
{
	unsigned char i;
	//me fijo si hay espacio
	if (p_numbers_speak < &numbers_speak[LAST_NUMBER_SPEAK])
	{
		//busco la primer posicion vacia y pongo el nuevo numero
		for (i = 0; i < LAST_NUMBER_SPEAK; i++)
		{
			if (numbers_speak[i] == 0)
			{
				numbers_speak[i] = new_number;
				i = LAST_NUMBER_SPEAK;
			}
		}
	}
}

//dice los numero en secuencia
//los numeros se cargan con VectorToSpeak()
void UpdateAudio (void)
{

	switch (audio_state)
	{
		case AUDIO_INIT:
			if (*p_numbers_speak != 0)		//ojo ver esto que no son char
				audio_state++;
			break;

		case AUDIO_UPDATE:
			//habilitar amplificador y pasarlo a audio
			//cargo el audio
			switch (*p_numbers_speak)
			{
				case 10:
					p_files_addr = &files.posi0;
					p_files_lenght = &files.lenght0;
					break;

				case 1:
					p_files_addr = &files.posi1;
					p_files_lenght = &files.lenght1;
					break;

				case 2:
					p_files_addr = &files.posi2;
					p_files_lenght = &files.lenght2;
					break;

				case 3:
					p_files_addr = &files.posi3;
					p_files_lenght = &files.lenght3;
					break;

				case 4:
					p_files_addr = &files.posi4;
					p_files_lenght = &files.lenght4;
					break;

				case 5:
					p_files_addr = &files.posi5;
					p_files_lenght = &files.lenght5;
					break;

				case 6:
					p_files_addr = &files.posi6;
					p_files_lenght = &files.lenght6;
					break;

				case 7:
					p_files_addr = &files.posi7;
					p_files_lenght = &files.lenght7;
					break;

				case 8:
					p_files_addr = &files.posi8;
					p_files_lenght = &files.lenght8;
					break;

				case 9:
					p_files_addr = &files.posi9;
					p_files_lenght = &files.lenght9;
					break;
			}

			Load16SamplesShort((unsigned short *)v_samples1, *p_files_addr + FILE_OFFSET);
			Load16SamplesShort((unsigned short *)v_samples2, *p_files_addr + FILE_OFFSET + 32);
			current_size = 64;
			update_samples = 0;

			Power_Ampli_Ena ();
			Ampli_to_Audio ();
			Usart1Send((char *) "-> To Audio\r\n");
			audio_state++;
			break;

		case AUDIO_SENDING:
			  if (update_samples)	//el update lo hace la interrupcion para la funcion seno
			  {
				  update_samples = 0;

				  if (current_size < (*p_files_lenght - FILE_OFFSET))
				  {
					  //LED_ON;
					  //DESDE ACA LEVANTO DE LA MEMORIA SST
					  //cargo el buffer que no esta en uso
					  if (buff_in_use == 1)
					  {
						  //cargo el buffer 2
						  Load16SamplesShort((unsigned short *)v_samples2, *p_files_addr + FILE_OFFSET + current_size);
					  }
					  else if (buff_in_use == 2)
					  {
						  //cargo el buffer 1
						  Load16SamplesShort((unsigned short *)v_samples1, *p_files_addr + FILE_OFFSET + current_size);
					  }
					  current_size += 32;
					  //LED_OFF;
				  }
				  else
				  {
					  //termine de enviar avanzo para ver si hay mas numeros
					  audio_state++;
				  }
			  }
			break;

		case AUDIO_CHECK_NEXT:
			p_numbers_speak++;
			if (*p_numbers_speak != '\0')		//ojo ver esto que no son char
				audio_state = AUDIO_UPDATE;
			else
				audio_state = AUDIO_FINISHING;

			break;

		case AUDIO_FINISHING:
			//llegue al final
			p_numbers_speak = numbers_speak;
			memset (numbers_speak, '\0', sizeof(numbers_speak));
			Power_Ampli_Disa ();
			Ampli_to_Sirena ();
			Usart1Send((char *) "-> To Sirena\r\n");
			audio_state = AUDIO_INIT;
			break;

		default:
			audio_state = AUDIO_INIT;
			break;
	}

}

#ifdef CON_MODIFICACION_DIODO_BATERIA
void UpdateBattery (void)
{
	if (!timer_battery)
	{
		timer_battery = 1;
		//reviso la pata

		if (ac_counter < 400)
		{
			ac_counter++;
			if (AC_PIN)
				bat_counter++;
		}
		else
		{
			ac_counter = 0;
			//me fijo si hago toggle
			if (bat_counter > 370)
			{
				//CON TENSION AC
				//titila rapido
				if (LED)
					LED_OFF;
				else
					LED_ON;
			}
			else
			{
				//SIN TENSION AC
				//titila lento
				if (!slow_toggle)
				{
					slow_toggle = 2;
					if (LED)
						LED_OFF;
					else
						LED_ON;
				}
				else
					slow_toggle--;

			}
			bat_counter = 0;
		}
	}
}
#endif

//muestra numeros en secuencia llamando a ShowNumbers
//los numeros se cargan con VectorToDisplay()
void UpdateDisplay (void)
{

	switch (display_state)
	{
		case DISPLAY_INIT:

			if (*p_numbers != '\0')		//ojo ver esto que no son char
				display_state++;
			else
			{
				//llegue al final
				p_numbers = numbers;
				memset (numbers, '\0', sizeof(numbers));
			}
			break;

		case DISPLAY_SENDING:
			ShowNumbers(*p_numbers);
			p_numbers++;
			display_state++;
			display_timeout = DISPLAY_TIMER_IN_ON;
			break;

		case DISPLAY_SHOWING:
			if (!display_timeout)
			{
				ShowNumbers(DISPLAY_NONE);
				display_timeout = DISPLAY_TIMER_IN_OFF;
				display_state++;
			}
			break;

		case DISPLAY_WAITING:
			if (!display_timeout)
			{
				display_state = DISPLAY_INIT;
			}
			break;

		default:
			display_state = DISPLAY_INIT;
			break;
	}

}

void SirenCommands(unsigned char command)
{
	siren_state = command;
}

void UpdateSiren (void)
{
	switch (siren_state)
	{
		case SIREN_INIT:
			break;

		case SIREN_SINGLE:
			//cargo frecuencia y ciclo de trabajo
			//ChangeAmpli(FREQ_1000HZ, DUTY_50_1000);
			ChangeAmpli(FREQ_800HZ, DUTY_50_800);
			Power_Ampli_Ena();
			siren_state = SIREN_SINGLE_RINGING;
			break;

		case SIREN_SINGLE_RINGING:
			break;

		case SIREN_SINGLE_CHOPP_ON:
			if (!siren_timeout)
			{
				//cargo frecuencia y ciclo de trabajo
				//ChangeAmpli(FREQ_1000HZ, DUTY_50_1000);
				ChangeAmpli(FREQ_800HZ, DUTY_50_800);
				Power_Ampli_Ena();
				siren_state = SIREN_SINGLE_CHOPP_OFF;
				siren_timeout = 1500;					//2 seg
			}
			break;

		case SIREN_SINGLE_CHOPP_OFF:
			if (!siren_timeout)
			{
				//vuelvo y dejo 500ms apagado
				Power_Ampli_Disa();
				siren_state--;
				siren_timeout = 500;
			}
			break;

		case SIREN_MULTIPLE_UP:
			siren_steps = 0;
			siren_state = SIREN_MULTIPLE_UP_B;
			Power_Ampli_Ena();
			break;

		case SIREN_MULTIPLE_UP_A:
			if (!siren_timeout)
			{
				//se termino el tiempo, cambio la frecuencia
				if (siren_steps)
					siren_steps--;
				siren_state = SIREN_MULTIPLE_UP_B;
			}
			break;

		case SIREN_MULTIPLE_UP_B:
			if (siren_steps)
			{
				freq_us = freq_us + SIREN_FREQ_STEP;
				siren_timeout = SIREN_STEP_TIMEOUT;
				ChangeAmpli(freq_us, freq_us >> 1);
				siren_state = SIREN_MULTIPLE_UP_A;
			}
			else
			{
				//empiezo el ciclo de nuevo
				siren_steps = SIREN_STEP_RELOAD;
				freq_us = SIREN_FIRST_FREQ;
				ChangeAmpli(freq_us, freq_us >> 1);
				siren_state = SIREN_MULTIPLE_UP_A;
				siren_timeout = SIREN_STEP_TIMEOUT;
			}
			break;

		case SIREN_CONFIRM_OK:
			siren_steps = 7;
			siren_state = SIREN_CONFIRM_OK_B;
			siren_timeout = SIREN_SHORT_TIMEOUT;
			Power_Ampli_Ena();
			break;

		case SIREN_CONFIRM_OK_A:
			if (!siren_timeout)
			{
				if (siren_steps)
				{
					//se termino el bip
					siren_steps--;
					siren_timeout = SIREN_SHORT_TIMEOUT;
					siren_state = SIREN_CONFIRM_OK_B;
					Power_Ampli_Ena();
				}
				else
				{
					//termino la secuencia
					siren_state = SIREN_TO_STOP;
				}
			}
			break;

		case SIREN_CONFIRM_OK_B:
			if (!siren_timeout)
			{
				//se termino el tiempo
				Power_Ampli_Disa();
				siren_state = SIREN_CONFIRM_OK_A;
				siren_timeout = SIREN_SHORT_TIMEOUT;
			}
			break;

		case SIREN_MULTIPLE_DOWN:
			siren_steps = 0;
			siren_state = SIREN_MULTIPLE_DOWN_B;
			Power_Ampli_Ena();
			break;

		case SIREN_MULTIPLE_DOWN_A:
			if (!siren_timeout)
			{
				//se termino el tiempo, cambio la frecuencia
				if (siren_steps)
					siren_steps--;
				siren_state = SIREN_MULTIPLE_DOWN_B;
			}
			break;

		case SIREN_MULTIPLE_DOWN_B:
			if (siren_steps)
			{
				freq_us = freq_us - SIREN_FREQ_STEP;
				siren_timeout = SIREN_STEP_TIMEOUT;
				ChangeAmpli(freq_us, freq_us >> 1);
				siren_state = SIREN_MULTIPLE_DOWN_A;
			}
			else
			{
				//empiezo el ciclo de nuevo
				siren_steps = SIREN_STEP_RELOAD;
				freq_us = SIREN_FIRST_FREQ;
				ChangeAmpli(freq_us, freq_us >> 1);
				siren_state = SIREN_MULTIPLE_DOWN_A;
				siren_timeout = SIREN_STEP_TIMEOUT;
			}
			break;

		case SIREN_SHORT:
			siren_timeout = SIREN_SHORT_TIMEOUT;
			siren_state = SIREN_SHL_TIMEOUT;
			Power_Ampli_Ena();
			break;

		case SIREN_HALF:
			siren_timeout = SIREN_HALF_TIMEOUT;
			siren_state = SIREN_SHL_TIMEOUT;
			Power_Ampli_Ena();
			break;

		case SIREN_LONG:
			siren_timeout = SIREN_LONG_TIMEOUT;
			siren_state = SIREN_SHL_TIMEOUT;
			Power_Ampli_Ena();
			break;

		case SIREN_SHL_TIMEOUT:
			if (!siren_timeout)
			{
				//se termino el tiempo
				siren_state = SIREN_TO_STOP;
			}
			break;

		case SIREN_TO_STOP:
		default:
			Power_Ampli_Disa();
			siren_state = SIREN_INIT;
			break;
	}
}

void BuzzerCommands(unsigned char command, unsigned char multiple)
{
	buzzer_state = command;
	buzz_multiple = multiple;
}


void UpdateBuzzer (void)
{
	switch (buzzer_state)
	{
		case BUZZER_INIT:
			break;

		case BUZZER_MULTIPLE_SHORT:
			if (buzz_multiple > 0)
			{
				BUZZER_ON;
				buzzer_state++;
				buzzer_timeout = TIM_BIP_SHORT;
			}
			else
				buzzer_state = BUZZER_TO_STOP;
			break;

		case BUZZER_MULTIPLE_SHORTA:
			if (!buzzer_timeout)
			{
				buzzer_state++;
				BUZZER_OFF;
				buzzer_timeout = TIM_BIP_SHORT_WAIT;
			}
			break;

		case BUZZER_MULTIPLE_SHORTB:
			if (!buzzer_timeout)
			{
				if (buzz_multiple)
					buzz_multiple--;

				buzzer_state = BUZZER_MULTIPLE_SHORT;
			}
			break;

		case BUZZER_MULTIPLE_HALF:
			if (buzz_multiple > 0)
			{
				BUZZER_ON;
				buzzer_state++;
				buzzer_timeout = TIM_BIP_HALF;
			}
			else
				buzzer_state = BUZZER_TO_STOP;
			break;

		case BUZZER_MULTIPLE_HALFA:
			if (!buzzer_timeout)
			{
				buzzer_state++;
				BUZZER_OFF;
				buzzer_timeout = TIM_BIP_HALF_WAIT;
			}
			break;

		case BUZZER_MULTIPLE_HALFB:
			if (!buzzer_timeout)
			{
				if (buzz_multiple)
					buzz_multiple--;

				buzzer_state = BUZZER_MULTIPLE_HALF;
			}
			break;

		case BUZZER_MULTIPLE_LONG:
			if (buzz_multiple > 0)
			{
				BUZZER_ON;
				buzzer_state++;
				buzzer_timeout = TIM_BIP_LONG;
			}
			else
				buzzer_state = BUZZER_TO_STOP;
			break;

		case BUZZER_MULTIPLE_LONGA:
			if (!buzzer_timeout)
			{
				buzzer_state++;
				BUZZER_OFF;
				buzzer_timeout = TIM_BIP_LONG_WAIT;
			}
			break;

		case BUZZER_MULTIPLE_LONGB:
			if (!buzzer_timeout)
			{
				if (buzz_multiple)
					buzz_multiple--;

				buzzer_state = BUZZER_MULTIPLE_LONG;
			}
			break;

		case BUZZER_TO_STOP:
		default:
			BUZZER_OFF;
			buzzer_state = BUZZER_INIT;
			break;
	}
}

void ConvertPositionToDisplay (unsigned short pos)
{
	unsigned char p0, p1, p2;

	p0 = pos / 100;

	if (p0 == 0)
		VectorToDisplay(10);
	else
		VectorToDisplay(p0);

	p1 = (pos - p0 * 100) / 10;

	if (p1 == 0)
		VectorToDisplay(10);
	else
		VectorToDisplay(p1);

	p2 = pos - p0 * 100 - p1 * 10;

	if (p2 == 0)
		VectorToDisplay(10);
	else
		VectorToDisplay(p2);

	VectorToDisplay(11);	//agrego punto
	VectorToDisplay(0);

}
/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.	ESTA ES LA QUE LLAMA SYSTICK CADA 1MSEG
  * @param  None
  * @retval None
  */


void TimingDelay_Decrement(void)
{
	if (TimingDelay != 0x00)
	{
		TimingDelay--;
	}

	if (wait_ms_var)
		wait_ms_var--;

	if (pilot_code)
		pilot_code--;

	//estos dos que siguen podrían ser el mismo
	if (interdigit_timeout)
		interdigit_timeout--;

	if (wait_for_code_timeout)
		wait_for_code_timeout--;

	if (switches_internals_timeout)
		switches_internals_timeout--;

	if (buzzer_timeout)
		buzzer_timeout--;

	if (display_timeout)
		display_timeout--;

	if (siren_timeout)
		siren_timeout--;

	if (timer_keypad_enabled)
		timer_keypad_enabled--;

#ifdef CON_MODIFICACION_DIODO_BATERIA
	if (timer_battery)
		timer_battery--;
#endif
	//cuenta 1 segundo
	if (button_timer_internal)
		button_timer_internal--;
	else
	{
		if (button_timer)
		{
			button_timer--;
			button_timer_internal = 1000;
		}
	}
}

