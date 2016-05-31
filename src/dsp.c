/*
 * dsp.c
 *
 *  Created on: Jul 8, 2014
 *      Author: Mariano
 */
#include "dsp.h"
#include "timer.h"
#include "derivative.h"
#include "hard.h"
#include "hidef.h"


//---- VARIABLES EXTERNAS ----//
//extern volatile unsigned char timer_overflow;

//---- VARIABLES GLOBALES ----//
//volatile unsigned int rti;
unsigned char update;
unsigned char cuadrante;
const unsigned char * psin;
const unsigned short * psin16;

#define SIZE_SIN32	32
const unsigned char sin32 [SIZE_SIN32] = {  127, 152, 175, 197, 216, 232, 243, 251,
											253, 251, 243, 232, 216, 197, 175, 152,
											127, 102, 79, 57, 38, 22, 11, 3,
											1, 3, 11, 22, 38, 57, 79, 102 };

#define SIZE_SIN8	8
const unsigned char sin8 [SIZE_SIN8] = { 127, 216, 253, 216, 127, 38, 1, 38 };
const unsigned char sin8_b [SIZE_SIN8] = { 127, 134, 141, 134, 127, 120, 113, 120 };

#define SIZE_SIN4	4
const unsigned char sin4 [SIZE_SIN4] = { 127, 255, 127, 0};
const unsigned short sin4_16 [SIZE_SIN4] = { 255, 511, 255, 0};

#pragma DATA_SEG MY_ZEROPAGE
short yrec [3] = {0};
short xrec [3] = {0};
int sumax = 0;
int sumay = 0;
#pragma DATA_SEG DEFAULT

#ifdef MA_8BITS
unsigned char vma [MA_LARGO_FILTRO + 1];
#endif

#ifdef MA_16BITS
unsigned short vma [MA_LARGO_FILTRO + 1];
#endif


//---- FUNCIONES DEL MODULO ----//
#ifdef MA_8BITS
unsigned char MA_8Bits (unsigned char medida)
{
	unsigned short total_ma, result_ma;
	unsigned char j;
	
	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7 
	total_ma = 0;
	vma[MA_LARGO_FILTRO] = medida; 
    for (j = 0; j < (MA_LARGO_FILTRO); j++)
    {
    	total_ma += vma[j + 1];
    	vma[j] = vma[j + 1];
    }
    result_ma = total_ma >> MA_DIVISOR;
    
    return (unsigned char) result_ma;
}
#endif

#ifdef MA_16BITS
unsigned short MA_16Bits (unsigned short medida)
{
	unsigned short total_ma, result_ma;
	unsigned char j;
	
	//Kernel mejorado ver 2
	//si el vector es de 0 a 7 (+1) sumo todas las posiciones entre 1 y 8, acomodo el nuevo vector entre 0 y 7 
	total_ma = 0;
	vma[MA_LARGO_FILTRO] = medida; 
    for (j = 0; j < (MA_LARGO_FILTRO); j++)
    {
    	total_ma += vma[j + 1];
    	vma[j] = vma[j + 1];
    }
    
    /*
    //REDONDEO
    //cuando tengo una diferencia de 1 voy a buscar redondear hacia arriba o abajo
    result_ma = total_ma >> (DIVISOR - 1);	//me quedo con el resto
    if (result_ma & 0x01)
    {
    	result_ma = total_ma >> DIVISOR;
    	result_ma += 1;
    }
    else
    	result_ma = total_ma >> DIVISOR;
    */
    result_ma = total_ma >> MA_DIVISOR;
    
    return result_ma;
}
#endif


void Gen_Sin(unsigned char vol, unsigned short freq)
{
	//genero senoidal con PWM
	//la freq minima es el tamaño de tabla por 4
	psin = sin32;
	//update = freq >> 2;
	//update <<= 3;
	update = 4;
	cuadrante = 0;
	
	//psin = sin8;
	psin = sin8_b;
}

/*
 * Recordar primero llamar a Gen_Sin para cargar las variables
 */
unsigned char Update_Sin(unsigned char prof)
{
	unsigned char dummy = 0;
	
	if (prof == 32)
	{
		/*
		switch (cuadrante)
		{
			case 0:
				if ((psin + update) < &sin[SIZE_SIN])
				{
					psin += update;
				}
				else
				{
					
				}
				break;
				
			case 1:
				break;
	
			case 2:
				break;
	
			case 3:
				break;
	
			default:
				cuadrante = 0;
				break;
		}
		*/
	
		//si tengo la senoidal completa
		
		if ((psin + update) < &sin32[SIZE_SIN32])
		{
			psin += update;
		}
		else
		{
			//me pase empiezo de nuevo
			dummy = &sin32[SIZE_SIN32] - psin;
			dummy = update - dummy;
			psin = sin32;
			psin += dummy;
		}
	}
	else if (prof == 9)
	{
		if (psin < &sin8_b[(SIZE_SIN8 - 1)])
			psin++;
		else
			psin = &sin8_b[0];
	}
	else if (prof == 8)
	{
		if (psin < &sin8[(SIZE_SIN8 - 1)])
			psin++;
		else
			psin = &sin8[0];
	}
	else if (prof == 4)
	{
		if (psin < &sin4[(SIZE_SIN4 - 1)])
			psin++;
		else
			psin = &sin4[0];
	}

	

	return *psin;	
}

unsigned short Update_Sin16(unsigned char prof)
{	
	if (prof == 4)
	{
		if (psin16 < &sin4_16[(SIZE_SIN4 - 1)])
			psin16++;
		else
			psin16 = &sin4_16[0];
	}
	return *psin16;	
}

void Gen_Sin4_16(unsigned short freq)
{
	//genero senoidal con PWM
	//la freq minima es el tamaño de tabla por 4
	
	psin16 = sin4_16;
	if (freq == 1400)
	{
		//tengo que cambiar fs a 11200 con 8 muestras por seno
		//TODO deberia hacerlo todo con el timer por ahora lo hago con MTIM
		//Gen_40K_Enabled();
		//TPM1MOD = VALOR_TPMMOD_EDGE;
		//TPM1C0V = VALOR_TPMC0V_EDGE;
			
		//CON MTIM
		MTIMCLK = 0x04;	//busclk = 20MHz / 16 
		MTIMMOD = 223;	//
		MTIMSC = 0x40; //int
	}
	else if (freq == 2300)
	{
		//CON MTIM
		MTIMCLK = 0x05;	//busclk = 20MHz / 32 = 625KHz
		MTIMMOD = 68;	//
		MTIMSC = 0x40; //int		
	}
}

void Gen_Sin4(unsigned short freq)
{
	//genero senoidal con PWM
	//la freq minima es el tamaño de tabla por 4
	
	psin = sin4;
	if (freq == 1400)
	{
		//tengo que cambiar fs a 11200 con 8 muestras por seno
		//TODO deberia hacerlo todo con el timer por ahora lo hago con MTIM
		//Gen_40K_Enabled();
		//TPM1MOD = VALOR_TPMMOD_EDGE;
		//TPM1C0V = VALOR_TPMC0V_EDGE;
			
		//CON MTIM
		MTIMCLK = 0x04;	//busclk = 20MHz / 16 
		MTIMMOD = 223;	//
		MTIMSC = 0x40; //int
	}
	else if (freq == 2300)
	{
		//CON MTIM
		MTIMCLK = 0x05;	//busclk = 20MHz / 32 = 625KHz
		MTIMMOD = 68;	//
		MTIMSC = 0x40; //int		
	}
}

void Gen_Sin8(unsigned short freq)
{
	//genero senoidal con PWM
	//la freq minima es el tamaño de tabla por 4
	
	psin = sin8;
	if (freq == 1400)
	{
		//tengo que cambiar fs a 11200 con 8 muestras por seno
		//TODO deberia hacerlo todo con el timer por ahora lo hago con MTIM
		//Gen_40K_Enabled();
		//TPM1MOD = VALOR_TPMMOD_EDGE;
		//TPM1C0V = VALOR_TPMC0V_EDGE;
			
		//CON MTIM
		MTIMCLK = 0x03;	//busclk = 20MHz / 8 = 2.5MHz
		//MTIMMOD = 223;	//cada tick 400ns, int en 89.2us fs -> 11210Hz
		MTIMMOD = 215;	//ajuste
		MTIMSC = 0x40; //int
	}
	else if (freq == 2300)
	{
		//CON MTIM
		MTIMCLK = 0x05;	//busclk = 20MHz / 32 = 625KHz
		MTIMMOD = 34;	//cada tick 1.6us, int en 54.4us fs -> 18382
		MTIMSC = 0x40; //int		
	}
}

unsigned short IIR (unsigned short newx)
{
	unsigned short sumax = 0;
	unsigned long sumay = 0;
//    y(i+grado) = 128*y(i+grado-1)-118*y(i+grado-2)+51*y(i+grado-3)-9*y(i+grado-4)+2*x(i+grado)+1*x(i+grado-1)+3*x(i+grado-2)+1*x(i+grado-3)+2*x(i+grado-4);
//    y(i+grado) = y (i+grado)/61;
	
	//implemento el recursivo de x
	xrec[4] = xrec[3];
	xrec[3] = xrec[2];
	xrec[2] = xrec[1];
	xrec[1] = xrec[0];
	xrec[0] = newx;
		
	sumax += 2*xrec[0];
	sumax += xrec[1];
	sumax += 3 * xrec[2];
	sumax += xrec[3];
	sumax += 2*xrec[4];
	sumax = sumax / 61;
	
	//implemento el recursivo de y
	yrec[4] = yrec[3];
	yrec[3] = yrec[2];
	yrec[2] = yrec[1];
	yrec[1] = yrec[0];

	sumay += 128 * yrec[1] / 61;
	sumay -= 118 * yrec[2] / 61;
	sumay += 51 * yrec[3] / 61;
	sumay -= 9 * yrec[4] / 61;

	sumay = sumay + sumax;

	//yrec[0] = (short) sumay;
        
	//si trunco yrec[0] oscila aprox 100hz y 240mV
//	if (sumay > 255)
//	{
//		yrec[0] = 255;
//	}
//	else
//		yrec[0] = sumay;
//	return yrec[0];

	//pruebo dejar yrec[0] como venga y truncar salida LO MISMO, TOCO ARRIBA DIVIDIDO POR 30 -> 31 (sigue oscilando cambia freq)
	yrec[0] = sumay;
	
	if (sumay > 255)
		return 255;
	else
		return sumay;

}

unsigned short IIR2 (unsigned short newx)
{
	short sumay2 = 0;
	//short sumax = 0;
	//short sumay = 0;
	sumax = 0;
	sumay = 0;
	
//	y(i+grado) = 21*x(i+grado)-21*x(i+grado-2)+255*y(i+grado-1)+166*y(i+grado-1)-213*y(i+grado-2);
//  y(i+grado) = y (i+grado)/256;


	//y[n] = 40 * x[n] - 40 * x[n-2] + 9 * y[n-1] + y[n-1] + 176 * y[n-2]
	//implemento el recursivo de x
	xrec[2] = xrec[1];
	xrec[1] = xrec[0];
	xrec[0] = newx;
		
	sumax += xrec[0];
	sumax -= xrec[2];
	sumax = sumax * 40;
	sumax >>= 8; 
	
	//implemento el recursivo de y
	yrec[2] = yrec[1];
	yrec[1] = yrec[0];

	sumay += 9 * yrec[1];
	sumay >>= 8;	
	sumay += yrec[1];
	sumay2 = 176 * yrec[2];
	sumay2 >>= 8;
	sumay -= sumay2;

	yrec[0] = sumax + sumay;

	/*
	//implemento el recursivo de x
	xrec[2] = xrec[1];
	xrec[1] = xrec[0];
	xrec[0] = newx;
		
	sumax += xrec[0];
	sumax -= xrec[2];
	sumax = sumax * 21;
	//sumax >>= 8; 
	sumax >>= 6;
	
	//implemento el recursivo de y
	yrec[2] = yrec[1];
	yrec[1] = yrec[0];

	sumay += 166 * yrec[1];
	//sumay >>= 8;
	sumay >>= 6;
	sumay += yrec[1];
	sumay += yrec[1];	//ver
	sumay += yrec[1];	//ver
	sumay += yrec[1];	//ver
	sumay2 = 213 * yrec[2];
	//sumay2 >>= 8;
	sumay2 >>= 6;
	sumay -= sumay2;

	yrec[0] = sumax + sumay;
	*/

	//como tiene un cero en w = 0, siempre le sumo un nivel de cotinua
	return (yrec[0] << 2) + 127;
}
