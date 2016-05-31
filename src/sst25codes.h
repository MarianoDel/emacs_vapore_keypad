/*
 * sst25codes.h
 *
 *  Created on: 03/05/2015
 *      Author: Mariano
 */

#ifndef SST25CODES_H_
#define SST25CODES_H_

#include "rws317.h"

#define OFFSET_CODES_256	0x2000
#define OFFSET_CODES_512	0x3000
#define OFFSET_CODES_768	0x4000
#define OFFSET_CODES_1024	0x5000

#define CODES_POSI_256		256
#define CODES_POSI_512		512
#define CODES_POSI_768		768
#define CODES_POSI_1024		1024

//unsigned short CheckCodeInMemory_SST (unsigned int);
//unsigned short CheckBaseCodeInMemory_SST (unsigned int);
unsigned char EraseAllMemory_SST(void);
unsigned int CheckIndexInMemory_SST (unsigned short);
unsigned short CheckBaseMask_SST (unsigned int, unsigned int);
unsigned char WriteCodeToMemory_SST(unsigned short, unsigned int);
void MemoryDump (unsigned int);
unsigned char CheckButtonInCode_SST (unsigned int, unsigned int);

#define CheckCodeInMemory_SST(X)	CheckBaseMask_SST((X), 0xFFFFFFFF)
#ifdef EV1527_BASE_CODE
#define CheckBaseCodeInMemory_SST(X)	CheckBaseMask_SST((X), EV1527_CODE_MASK)
#define CheckButtonInCode(X)	CheckButtonInCode_SST((X), EV1527_BUTTON_MASK)
#endif
#ifdef HT6P20B2_BASE_CODE
#define CheckBaseCodeInMemory_SST(X)	CheckBaseMask_SST((X), HT6P20B2_CODE_MASK)
#define CheckButtonInCode(X)	CheckButtonInCode_SST((X), HT6P20B2_BUTTON_MASK)
#endif

#endif /* SST25CODES_H_ */
