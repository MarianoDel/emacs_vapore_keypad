/*
 * dsp.h
 *
 *  Created on: Jul 8, 2014
 *      Author: Mariano
 */

#ifndef DSP_H_
#define DSP_H_

//--- Definiciones para el filtro Moving Average de 8 o de 16 bits ---//
#define MA_8BITS
//#define MA_16BITS
#define MA_LARGO_FILTRO 8
#define MA_DIVISOR      3   


void Gen_Sin(unsigned char, unsigned short);
unsigned char Update_Sin(unsigned char);
void Gen_Sin8(unsigned short);
void Gen_Sin4(unsigned short);
void Gen_Sin4_16(unsigned short);
unsigned short Update_Sin16(unsigned char);
unsigned short IIR2 (unsigned short);

unsigned char MA_8Bits (unsigned char);
unsigned short MA_16Bits (unsigned short);

#endif /* DSP_H_ */
