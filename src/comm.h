/*
 * comm.h
 *
 *  Created on: Oct 29, 2013
 *      Author: Mariano
 */

#ifndef COMM_H_
#define COMM_H_

#define GET_PARAMS	10
#define GET_TEMP 11
#define GET_GAUSS 12
#define CHANNEL	13
#define SET_DISPLAY	14
#define CMD_DISPLAY	15
#define KEEP_ALIVE	16

#define ERROR	50

//funciones
unsigned char InterpretarMsg (unsigned char, char *);
void AnalizarMsg (char *);
unsigned char Check_Own_Channel (char *);
unsigned short ToInt3 (char *);

#endif /* COMM_H_ */
