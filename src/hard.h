//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F103
// ##
// #### HARD.H ################################
//---------------------------------------------
#ifndef _HARD_H_
#define _HARD_H_


//------ FUNCIONAMIENTO GENERAL DEL EQUIPO ------//
//----------- Defines For Configuration -------------
// VER HARD.H LO QUE CORRESPONDE A HARDWARE
// VER RWS317.H LO QUE TIENE QUE VER CON CODIGOS Y CONTROLES
#define CON_BLOQUEO_DE_KEYPAD
#define CON_MODIFICACION_DIODO_BATERIA
// #define WITH_WELCOME_CODE_ON_DISPLAY

// #define PROGRAMA_NORMAL
// #define PROGRAMA_DE_BUCLE
// #define PROGRAMA_FACTORY_TEST
#define PROGRAMA_DE_GESTION

//----------- Hardware Board Version -------------
#define VER_1_3

//---- Configuration for Hardware Versions -------
#ifdef VER_1_3
#define HARDWARE_VERSION_1_3
#define SOFTWARE_VERSION_2_0
#endif


//---- Features Configuration ----------------

//------ Configuration for Firmware-Channels -----

//--- Hardware Welcome Code ------------------//
#ifdef HARDWARE_VERSION_1_3
#define HARD "Hardware V: 1.3\r\n"
#endif

//--- Software Welcome Code ------------------//
#ifdef SOFTWARE_VERSION_2_0
#define SOFT "Software V: 2.0\r\n"
#endif

//--- Type of Program Announcement ----------------
#ifdef PROGRAMA_NORMAL
#define KIND_OF_PROGRAM "Panel Normal\r\n"
#endif
#ifdef PROGRAMA_DE_BUCLE
#define KIND_OF_PROGRAM "Panel con Programa para Bucle\r\n"
#endif
#ifdef PROGRAMA_FACTORY_TEST
#define KIND_OF_PROGRAM "Panel con Programa de testeo en fabrica\r\n"
#endif
#ifdef PROGRAMA_DE_GESTION
#define KIND_OF_PROGRAM "Programa de Gestion para Grabar SST\r\n"
#endif
//---- End of Program Configuration ----------


#define SPI_MASTER
//para GPIO 1 solo bit uso Port bit set/reset register (GPIOx_BSRR) (x=A..G)

//------ OJO PARA PRUEBAS
//------ NO LLEGO EL MICRO FINAL
//GPIOA pin0
//GPIOA pin1

//GPIOA pin2
#define OE ((GPIOA->ODR & 0x0004) == 0)
#define OE_OFF GPIOA->BSRR = 0x00000004
#define OE_ON GPIOA->BSRR = 0x00040000

//GPIOA pin3
#define WP ((GPIOA->ODR & 0x0008) == 0)
#define WP_OFF GPIOA->BSRR = 0x00000008
#define WP_ON GPIOA->BSRR = 0x00080000

//GPIOA pin4

//GPIOA pin5
#define PS ((GPIOA->ODR & 0x0020) == 0)
#define PS_OFF GPIOA->BSRR = 0x00000020
#define PS_ON GPIOA->BSRR = 0x00200000

//GPIOA pin6
#define CE ((GPIOA->ODR & 0x0040) == 0)
#define CE_OFF GPIOA->BSRR = 0x00000040
#define CE_ON GPIOA->BSRR = 0x00400000

//GPIOA pin7
//GPIOA pin8
//GPIOA pin9

//GPIOA pin11
#define F5PLUS ((GPIOA->ODR & 0x0800) != 0)
#define F5PLUS_ON GPIOA->BSRR = 0x00000800
#define F5PLUS_OFF GPIOA->BSRR = 0x08000000

//GPIOA pin12
#define BUZZER ((GPIOA->ODR & 0x1000) != 0)
#define BUZZER_ON GPIOA->BSRR = 0x00001000
#define BUZZER_OFF GPIOA->BSRR = 0x10000000

//GPIOA pin13
//GPIOA pin14
//GPIOA pin15
#define AC_PIN ((GPIOA->IDR & 0x8000) != 0)

//GPIOB pin0
#define FPLUS ((GPIOB->ODR & 0x0001) != 0)
#define FPLUS_ON GPIOB->BSRR = 0x00000001
#define FPLUS_OFF GPIOB->BSRR = 0x00010000

//GPIOB pin1
#define F12PLUS ((GPIOB->ODR & 0x0002) != 0)
#define F12PLUS_ON GPIOB->BSRR = 0x00000002
#define F12PLUS_OFF GPIOB->BSRR = 0x00020000

//GPIOB pin6
#define LED ((GPIOB->ODR & 0x0040) != 0)
#define LED_ON GPIOB->BSRR = 0x00000040
#define LED_OFF GPIOB->BSRR = 0x00400000


#define CH_IN_TEMP ADC_Channel_0
#define LAST_NUMBER		6
#define LAST_NUMBER_SPEAK		5

//--- TIMEOUTS DEL PROGRAMA ----//
#define TIM_BIP_SHORT		50
#define TIM_BIP_SHORT_WAIT		100
#define TIM_BIP_HALF		200
#define TIM_BIP_HALF_WAIT		500
#define TIM_BIP_LONG		1200
#define TIM_BIP_LONG_WAIT		1500


//--- FIN TIMEOUTS          ----//


//ESTADOS DEL PROGRAMA PRINCIPAL PARA TESTEO
enum TestStateEnum
{
	TEST_INIT,
	TEST_CHECK_BUZZER,
	TEST_CHECK_MEMORY_WRITE,
	TEST_CHECK_MEMORY_READ,
	TEST_CHECK_KEYPAD,
	TEST_ERROR
} TestState;


//ESTADOS DEL PROGRAMA PRINCIPAL
#define MAIN_INIT			0
#define MAIN_MAIN			1
#define MAIN_IN_ALARM		2
#define MAIN_TO_UNLOCK		3
#define MAIN_UNLOCK			4

#define MAIN_TO_DEL_CODE				10
#define MAIN_TO_DEL_REMOTE_CODE			11
#define MAIN_TO_SAVE_AT_LAST			12
#define MAIN_TO_SAVE_REMOTE_AT_LAST		13
#define MAIN_TO_CHANGE_USER_PASSWORD	14

#define MAIN_TO_MAIN_CANCEL		20
#define MAIN_TO_MAIN_OK			21
#define MAIN_TO_MAIN_TIMEOUT		22
#define MAIN_TO_MAIN_WAIT_5SEGS         23
#define MAIN_TO_MAIN_WAIT_5SEGSA        24
#define MAIN_TO_MASS_ERASE_AT_LAST	25

// #define MAIN_TO_MASS_ERASE0		30
// #define MAIN_TO_MASS_ERASE1		31




//ESTADOS DEL KEYPAD REMOTO
#define RK_NONE						0
#define RK_RECEIVING_A				1
#define RK_RECEIVING_B				2
#define RK_RECEIVING_C				3
#define RK_RECEIVING_D				4
#define RK_RECEIVING_E				5
#define RK_RECEIVING_F				6
#define RK_NUMBER_FINISH			8
#define RK_CANCEL					9
#define RK_TIMEOUT					10
#define RK_MUST_BE_CONTROL			11


//ESTADOS DEL BUZZER
#define BUZZER_INIT		0
#define BUZZER_TO_STOP		10

#define BUZZER_MULTIPLE_LONG			40
#define BUZZER_MULTIPLE_LONGA			41
#define BUZZER_MULTIPLE_LONGB			42

#define BUZZER_MULTIPLE_HALF			50
#define BUZZER_MULTIPLE_HALFA			51
#define BUZZER_MULTIPLE_HALFB			52

#define BUZZER_MULTIPLE_SHORT			60
#define BUZZER_MULTIPLE_SHORTA			61
#define BUZZER_MULTIPLE_SHORTB			62

//ESTADOS DE LA ALARMA
#define ALARM_START				0
#define ALARM_BUTTON1			10
#define ALARM_BUTTON1_A			11
#define ALARM_BUTTON1_B			12
#define ALARM_BUTTON1_C			13
#define ALARM_BUTTON1_D			14
#define ALARM_BUTTON1_E			15
#define ALARM_BUTTON1_FINISH	16
#define ALARM_BUTTON1_FINISH_B	17

#define ALARM_NO_CODE			100

#define ALARM_BUTTON2		20
#define ALARM_BUTTON2_A		21
#define ALARM_BUTTON2_B		22
#define ALARM_BUTTON2_C		23
#define ALARM_BUTTON2_D			24
#define ALARM_BUTTON2_E			25
#define ALARM_BUTTON2_FINISH	26

#define ALARM_BUTTON3			30
#define ALARM_BUTTON3_A			31
#define ALARM_BUTTON3_B			32
#define ALARM_BUTTON3_C			33
#define ALARM_BUTTON3_D			34
#define ALARM_BUTTON3_E			35
#define ALARM_BUTTON3_FINISH	36
#define ALARM_BUTTON3_FINISH_B	37

#define ALARM_BUTTON4			40
#define ALARM_BUTTON4_A			41
#define ALARM_BUTTON4_FINISH	42



//ESTADOS DEL AUDIO
#define AUDIO_INIT					0
#define AUDIO_UPDATE				1
#define AUDIO_SENDING				2
#define AUDIO_CHECK_NEXT			3
#define AUDIO_FINISHING				4

//COMANDOS DEL BUZZER	(tienen que ser los del estado de arriba)
#define BUZZER_STOP_CMD		10
#define BUZZER_LONG_CMD		40
#define BUZZER_HALF_CMD		50
#define BUZZER_SHORT_CMD	60

//ESTADOS DE LA SIRENA
#define SIREN_INIT				0
#define SIREN_SINGLE			1
#define SIREN_SINGLE_RINGING	2
#define SIREN_SINGLE_CHOPP_ON	3
#define SIREN_SINGLE_CHOPP_OFF	4
#define SIREN_MULTIPLE_UP		5
#define SIREN_MULTIPLE_UP_A		6
#define SIREN_MULTIPLE_UP_B		7
#define SIREN_TO_STOP			8
#define SIREN_MULTIPLE_DOWN		9
#define SIREN_MULTIPLE_DOWN_A	10
#define SIREN_MULTIPLE_DOWN_B	11
#define SIREN_SHORT				12
#define SIREN_HALF				13
#define SIREN_LONG				14
#define SIREN_SHL_TIMEOUT		15
#define SIREN_CONFIRM_OK		16
#define SIREN_CONFIRM_OK_A		17
#define SIREN_CONFIRM_OK_B		18


//COMANDOS DE LA SIRENA	(tienen que ser los del estado de arriba)
#define SIREN_SINGLE_CMD			SIREN_SINGLE
#define SIREN_SINGLE_CHOPP_CMD		SIREN_SINGLE_CHOPP_ON
#define SIREN_STOP_CMD				SIREN_TO_STOP
#define SIREN_MULTIPLE_UP_CMD		SIREN_MULTIPLE_UP
#define SIREN_MULTIPLE_DOWN_CMD		SIREN_MULTIPLE_DOWN
#define SIREN_SHORT_CMD				SIREN_SHORT
#define SIREN_HALF_CMD				SIREN_HALF
#define SIREN_LONG_CMD				SIREN_LONG
#define SIREN_CONFIRM_OK_CMD		SIREN_CONFIRM_OK

//TIEMPOS DE LA SIRENA POLICIAL
//#define SIREN_FIRST_FREQ	800		//este es el tiempo en us (1/Hz) donde arranca la sirena
#define SIREN_FIRST_FREQ	1400		//este es el tiempo en us (1/Hz) donde arranca la sirena
#define SIREN_STEP_TIMEOUT  15		//tiempo en el que se mantiene una frecuencia antes de avanzar
#define SIREN_STEP_RELOAD	40
#define SIREN_FREQ_STEP		10		//incremento en us entre frecuencias (OJO usar siempre par, el pwm es /2 )
									//SIREN_FIRST_FREQ + SIREN_STEP_RELOAD * SIREN_FREQ_STEP = ultima frecuencia
									//freq original = 800 -> 1250Hz
									//800 + 50 * 20 = 1800 -> 555Hz

									//el tiempo en que tarda en llegar de una a la otra es:
									//SIREN_STEP_RELOAD * SIREN_STEP_TIMEOUT
									//50 * 80ms = 4s

#define SIREN_SHORT_TIMEOUT	200
#define SIREN_HALF_TIMEOUT	400
#define SIREN_LONG_TIMEOUT	1000


enum end_states
{
	WORKING = 1,
	END_OK,
	END_ERROR,
	END_TIMEOUT
};

#endif /* _HARD_H_ */

//--- end of file ---//

