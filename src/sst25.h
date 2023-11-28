/*----------------------------------------------

//                                      sst25vf016.h

//      Serial Flash driver

//      Arthur Bemann 01/05/2011

//-----------------------------------------------

//

//      Description:

//      Driver to read and write to a sst25vf016 memmory

//

//-----------------------------------------------

//

//      Requeriments:

//      SPI2

//      3 Remapable pins

//      1 General I/O pin

//      Main Clock must be 80MHz

//

//-----------------------------------------------

*/

#ifndef SST25_H
#define SST25_H

#include "hard.h"

#define SST25_SPRE              0x6                     // Secondary Prescaler 2:1
#define SST25_PPRE              0x2                     // Primary Prescaler 4:1
#define SST25_SIZE              0x200000        //      16Mbit flash


//---------------------- SST25 Instruction Set -----------------------
#define SST25_WREN                      0x06    // Write Enable
#define SST25_WRDI                      0x04    // Write Disable
#define SST25_WRSR                      0x01    // Write Status Register
#define SST25_CHIP_ERASE        0x60    // Full Chip Erase
#define SST25_SECTOR_ERASE		        0x20    // 4K Erase
#define SST25_RDSR                      0x05    // Read Status Register
#define SST25_READ                      0x03    // Read memory
#define SST25_WRITE                     0x02    // Write memory
#define SST25_AAIP                      0xAD    // Auto Address Increment Programming
#define SST25_JEDEC                     0x9F    // JEDEC ID read
#define SST25_RDID                      0x90    // Read ID

#define SST25_UNPROTECT 0x00            // Byte that must be transfered to the Status register to unprotect the memory
#define SST5_BUSY               0x01            //Mask for the Busy Bit on the status register



//------------------------------ MACROS -------------------------------
#define NVM_On          CE_ON
#define NVM_Off         CE_OFF

#define readSPI2()      SPI_Receive_Single()
#define busyNVM()   (readStatusNVM()& SST5_BUSY)// Returns the state of the Busy bit on the status register


//-------- Functions -------------
void initNVM(void);
void clearNVM(void);
void Clear4KNVM(unsigned int address);
unsigned char readNVM(unsigned int addr);
void writeNVM(unsigned char data,unsigned int address);
void writeBufNVM(unsigned char *buf,unsigned short len,unsigned int address);
void writeBufNVM16u(unsigned short *buf,unsigned short len,unsigned int address);
unsigned char readStatusNVM(void);
void readBufNVM(unsigned char *buf, unsigned short len, unsigned int address);
void readBufNVM8u(unsigned char *buf, unsigned short len, unsigned int address);
unsigned char readJEDEC (void);
void getJEDEC (unsigned char * jedec);

void writePageNVM(unsigned char *buf, unsigned short len, unsigned int address);
void writeBufferNVM (unsigned char *buff, unsigned short len, unsigned int address);

unsigned char readStatus2NVM(void);
unsigned char readStatus3NVM(void);

#endif
