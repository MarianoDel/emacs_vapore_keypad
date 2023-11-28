/*----------------------------------------------

//                                      sst25vf016.c

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

#include "sst25.h"                 // Self definitions
#include "spi.h"
#include "stm32f0xx.h"






typedef enum {
    SST25_DETECTED = 0,
    W25_DETECTED

} detected_mem_e;


//------------------------ PRIVATE PROTOTYPES --------------------------
void unprotectNVM(void);
unsigned char readStatusNVM(void);
void writeSPI2 (unsigned char);
void writeMultiplePageNVM (unsigned char *buf, unsigned short len, unsigned int address);


// Globals ---------------------------------------------------------------------
detected_mem_e nvm_detected = SST25_DETECTED;


//--------------------------- FUNCTIONS --------------------------------


//--------------- initNVM --------------------

// Initialize SPI to talk with the memmory
/*
void initNVM(void)

{

        SPI2CON1bits.DISSCK     = 0;    // Internal clock enabled

        SPI2CON1bits.DISSDO = 0;        // SDO controlled by module

        SPI2CON1bits.MODE16     = 0;    // Byte wide communication

        SPI2CON1bits.SMP        = 1;    // Input sampled at end of data time

        SPI2CON1bits.CKE        = 1;    // Data changes from high to low clock

        SPI2CON1bits.CKP        = 0;

        SPI2CON1bits.MSTEN      = 1;    // Master mode enabled

        SPI2CON1bits.SSEN       = 0;    // SS pin is controlled by port



        SPI2CON1bits.SPRE       = SST25_SPRE;

        SPI2CON1bits.PPRE       = SST25_PPRE;



        SPI2CON2bits.FRMEN      = 0;    // Framed SPIx is disabled



        SST25VF_CS_TRIS         = 0;    // Configure Chip Select pin

        SST25VF_CS                      = 1;

        SST25VF_SDO;                            // Configure SDO pin

        SST25VF_SDI;                            // Configure SDI pin

        SST25VF_SCK;                            // Configure SCK pin





        SPI2STAT            = 0;

        SPI2STATbits.SPISIDL= 0;        // Operational in idle mode

        _SPI2IF                         = 0;

        SPI2STATbits.SPIEN      = 1;    // Enable the module

}
*/



//---------------- writeNVM -------------------

// write one byte to the NVM

//int writeSPI2(int data)
void writeSPI2(unsigned char data)
{
	//return (Send_Receive_SPI(data));
	SPI_Send_Single(data);
}



//-------------- readStatusNVM ----------------

// Read the Status Register from the memory

unsigned char readStatusNVM(void)
{
	unsigned char data;

	NVM_On;
	writeSPI2(SST25_RDSR);
	data = readSPI2();
	NVM_Off;

	return data;
}


unsigned char readStatus2NVM(void)
{
    unsigned char data;

    NVM_On;
    writeSPI2(0x35);
    data = readSPI2();
    NVM_Off;

    return data;
}


unsigned char readStatus3NVM(void)
{
    unsigned char data;

    NVM_On;
    writeSPI2(0x15);
    data = readSPI2();
    NVM_Off;

    return data;
}


void writeStatus2NVM (unsigned char data)
{
    // Enable Write for Non-Volatile bits
    // NVM_On;
    // writeSPI2(SST25_WREN);
    // NVM_Off;

    // Enable Write for Volatile bits
    NVM_On;
    writeSPI2(0x50);
    NVM_Off;
    
    // Write to Status Register 2 on 0x35
    NVM_On;
    writeSPI2(0x31);
    writeSPI2(data);
    NVM_Off;

    // Wait until write to complete
    while(busyNVM());    
}


void writeStatus3NVM (unsigned char data)
{
    // Enable Write
    NVM_On;
    writeSPI2(0x50);
    NVM_Off;

    // Write to Status Register 3 on 0x11
    NVM_On;
    writeSPI2(0x11);
    writeSPI2(data);
    NVM_Off;

    // Wait until write to complete
    while(busyNVM());
}





//------------ unprotectNVM ------------------

// Disable the write protection from the NVM

void unprotectNVM(void)

{

                // Enable Write

                NVM_On;

                writeSPI2(SST25_WREN);

                NVM_Off;



                // Unprotect Memory

                NVM_On;

                writeSPI2(SST25_WRSR);

                writeSPI2(SST25_UNPROTECT);

                NVM_Off;

}



//-------------- clearNVM ------------------
// Clear the entire memory
// for both W25 & SST25
void clearNVM(void)
{
    if (nvm_detected == SST25_DETECTED)
    {
        // Disable Write Protection
        unprotectNVM();
    }

    // Enable Write
    NVM_On;
    writeSPI2(SST25_WREN);
    NVM_Off;

    // Chip Erase
    NVM_On;
    writeSPI2(SST25_CHIP_ERASE);
    NVM_Off;

    // Wait until Erasing is complete
    while(busyNVM());
}


// for W25
// void clearNVM (void)
// {
//     // Enable Write
//     NVM_On;
//     writeSPI2(SST25_WREN);
//     NVM_Off;

//     // Chip Erase
//     NVM_On;
//     writeSPI2(SST25_CHIP_ERASE);
//     NVM_Off;

//     // Wait until Erasing is complete
//     while(busyNVM());
// }


// for SST25
// void clearNVM_Unprotect (void)
// {
//     // Disable Write Protection
//     unprotectNVM();

//     // Enable Write
//     NVM_On;
//     writeSPI2(SST25_WREN);
//     NVM_Off;

//     // Chip Erase
//     NVM_On;
//     writeSPI2(SST25_CHIP_ERASE);
//     NVM_Off;

//     // Wait until Erasing is complete
//     while(busyNVM());
// }

//-------------- clear4KNVM ------------------
// Clear 4K at specified address

// for both W25 & SST25
void Clear4KNVM (unsigned int address)
{
    if (nvm_detected == SST25_DETECTED)
    {
        // Disable Write Protection
        unprotectNVM();
    }

    // Enable Write
    NVM_On;
    writeSPI2(SST25_WREN);
    NVM_Off;

    // 4K Erase
    NVM_On;
    writeSPI2(SST25_SECTOR_ERASE);

    // Send Address to be erase
    writeSPI2((address & 0x00FF0000) >> 16);	// MSB
    writeSPI2((address & 0x0000FF00) >> 8);
    writeSPI2((address & 0x000000FF) );			// LSB

    NVM_Off;

    // Wait until Erasing is complete
    while(busyNVM());
}


// for W25
// void Clear4KNVM (unsigned int address)
// {
//     // Enable Write
//     NVM_On;
//     writeSPI2(SST25_WREN);
//     NVM_Off;

//     // 4K Erase
//     NVM_On;
//     writeSPI2(SST25_SECTOR_ERASE);

//     // Send Address to be erase
//     writeSPI2((address & 0x00FF0000) >> 16);	// MSB
//     writeSPI2((address & 0x0000FF00) >> 8);
//     writeSPI2((address & 0x000000FF) );			// LSB

//     NVM_Off;

//     // Wait until Erasing is complete
//     while(busyNVM());
// }


// for SST25
// void Clear4KNVM_Unprotect (unsigned int address)
// {
//     // Disable Write Protection
//     unprotectNVM();

//     // Enable Write
//     NVM_On;
//     writeSPI2(SST25_WREN);
//     NVM_Off;

//     // 4K Erase
//     NVM_On;
//     writeSPI2(SST25_SECTOR_ERASE);

//     // Send Address to be erase
//     writeSPI2((address & 0x00FF0000) >> 16);	// MSB
//     writeSPI2((address & 0x0000FF00) >> 8);
//     writeSPI2((address & 0x000000FF) );			// LSB

//     NVM_Off;

//     // Wait until Erasing is complete
//     while(busyNVM());
// }


//--------------- readJEDEC ----------------
// for both W25 & SST25
unsigned char readJEDEC(void)
{
    unsigned char answer = 0;
    unsigned char buf[3];

    // Send Read Command
    NVM_On;

    writeSPI2(SST25_JEDEC);

    // receive data
    buf[0] = readSPI2();
    buf[1] = readSPI2();
    buf[2] = readSPI2();
    NVM_Off;

    // for winbond 0xEF 0x40 0x15
    if ((buf[0] == 0xEF) && (buf[1] == 0x40) && (buf[2] == 0x15))
    {
        nvm_detected = W25_DETECTED;
        answer = 1;
    }
    else if (((buf[0] == 0xBF) && (buf[1] == 0x25) && (buf[2] == 0x41)) ||
             ((buf[0] == 0xBF) && (buf[1] == 0x25) && (buf[2] == 0x4A)))
    {
        // 16Mbit (0x41h)  or 32Mbit memory (0x4Ah)
        nvm_detected = SST25_DETECTED;        
        answer = 1;
    }

    return answer;
}


// unsigned char readJEDEC(void)
// {
//     unsigned char buf[3];

//     // Send Read Command
//     NVM_On;

//     writeSPI2(SST25_JEDEC);

//     // receive data
//     buf[0] = readSPI2();
//     buf[1] = readSPI2();
//     buf[2] = readSPI2();
//     NVM_Off;

//     // for winbond 0xEF 0x40 0x15
//     // for microchip 0xBF 0x25 ... 16Mbit (0x41h)  or 32Mbit memory (0x4Ah)
//     if (((buf[0] == 0xEF) && (buf[1] == 0x40) && (buf[2] == 0x15)) ||
//         ((buf[0] == 0xBF) && (buf[1] == 0x25) && (buf[2] == 0x41)) ||
//         ((buf[0] == 0xBF) && (buf[1] == 0x25) && (buf[2] == 0x4A)))
//     {
//         return 1;
//     }

//     return 0;
// }


// For SST 16Mbit (0x41h)  or 32Mbit memory (0x4Ah)
void getJEDEC (unsigned char * jedec)
{
    // Send Read Command
    NVM_On;

    writeSPI2(SST25_JEDEC);

    // receive data
    *(jedec + 0 ) = readSPI2();
    *(jedec + 1 ) = readSPI2();
    *(jedec + 2 ) = readSPI2();

    NVM_Off;

}


//--------------- readNVM ----------------

// Reads one byte from the NVM

unsigned char readNVM(unsigned int address)
{
	unsigned char data;

    // Send Read Command
	NVM_On;

    writeSPI2(SST25_READ);

    // Send Address to be read
    writeSPI2((address & 0x00FF0000) >> 16);	// MSB
    writeSPI2((address & 0x0000FF00) >> 8);
    writeSPI2((address & 0x000000FF) );			// LSB

    // receive data
    data = readSPI2();

    NVM_Off;

    return data;
}



//--------------- readBufNVM ----------------

// Reads a series of byte from the NVM to a buffer

// Address must be even, because of aligniment

// little-endian ( LSB first)

void readBufNVM(unsigned char *buf, unsigned short len, unsigned int address)
{
	// Send Read Command
	NVM_On;

	writeSPI2(SST25_READ);

	// Send Address to be read
	writeSPI2((address & 0x00FF0000) >> 16);                // MSB
	writeSPI2((address & 0x0000FF00) >> 8);
	writeSPI2((address & 0x000000FF) );                             // LSB

	// Receive requested data
	while ((len--) > 0)
	{
		// receive data
        *buf     = readSPI2();
        (*buf++)+= readSPI2() << 8;
    }

	NVM_Off;
}

void readBufNVM8u(unsigned char *buf, unsigned short len, unsigned int address)
{
	// Send Read Command
	NVM_On;

	writeSPI2(SST25_READ);

	// Send Address to be read
	writeSPI2((address & 0x00FF0000) >> 16);                // MSB
	writeSPI2((address & 0x0000FF00) >> 8);
	writeSPI2((address & 0x000000FF) );                             // LSB

	// Receive requested data
	while ((len--) > 0)
	{
		// receive data
        *buf     = readSPI2();
        buf++;
    }

	NVM_Off;
}


//-------------- writeNVM ------------------

// Write one byte to the NVM

// The SST25 must not be write protected

void writeNVM(unsigned char data, unsigned int address)
{
	// Enable Writes

                NVM_On;

                writeSPI2(SST25_WREN);

                NVM_Off;



                NVM_On;

                writeSPI2(SST25_WRITE); // Send Write Command



                // Send Address to be written

                writeSPI2((address & 0x00FF0000) >> 16);                // MSB

                writeSPI2((address & 0x0000FF00) >> 8);

                writeSPI2((address & 0x000000FF) );                             // LSB



                // Send data to be written

                writeSPI2(data);

                NVM_Off;



                // Wait while writting

                while(busyNVM());



                // Disables writings

                NVM_On;

                writeSPI2(SST25_WRDI);

                NVM_Off;

}



//------------- writeBufNVM -----------------

// Write an entire buffer to the NVM, starting

// at the position passed as arg. .

// Address must be even, because of aligniment

// little-endian ( LSB first)

void writeBufNVM(unsigned char *buf, unsigned short len, unsigned int address)

{

                // Enable Writes

                NVM_On;

                writeSPI2(SST25_WREN);

                NVM_Off;



                // Send Sequential Write Command

                NVM_On;

                writeSPI2(SST25_AAIP);



                // Send Address to be written

                writeSPI2((address & 0x00FF0000) >> 16);                // MSB

                writeSPI2((address & 0x0000FF00) >> 8);

                writeSPI2((address & 0x000000FF) );                             // LSB



                // Send first word to be Written

                writeSPI2( (*buf)       & 0xFF);                                // Send word to be written

                writeSPI2(((*buf++)>>8) & 0xFF);

                NVM_Off;

                len--;

                while(busyNVM());               // Wait until write is complete



                while((len--)>0)

                {

                        // Send Sequential Write Command

                        NVM_On;

                        writeSPI2(SST25_AAIP);

                        writeSPI2( (*buf)       & 0xFF);                        // Send word to be written

                        writeSPI2(((*buf++)>>8) & 0xFF);

                        NVM_Off;

                        while(busyNVM());               // Wait until write is complete

                }



                NVM_On;

                writeSPI2(SST25_WRDI);  // Disables future writings

                NVM_Off;

}

//El AAI necesita que se escriban 16 bits
void writeBufNVM16u(unsigned short *buf, unsigned short len, unsigned int address)
{
	// Enable Writes
	NVM_On;
	writeSPI2(SST25_WREN);
	NVM_Off;

	// Send Sequential Write Command
	NVM_On;
	writeSPI2(SST25_AAIP);

	// Send Address to be written
	writeSPI2((address & 0x00FF0000) >> 16);                // MSB
	writeSPI2((address & 0x0000FF00) >> 8);
	writeSPI2((address & 0x000000FF) );                     // LSB

	// Send Word to be Written
	writeSPI2((*buf) & 0x00FF);                               // Send byte to be written
    writeSPI2((*buf>>8) & 0x00FF);
    buf++;
	len--;

	NVM_Off;

	while (busyNVM());               // Wait until write is complete

	while ((len--) > 0)
	{
		// Send Sequential Write Command
		NVM_On;
		writeSPI2(SST25_AAIP);

		writeSPI2((*buf) & 0x00FF);                        // Send byte to be written
		writeSPI2((*buf>>8) & 0x00FF);
		buf++;
		NVM_Off;

		while(busyNVM());               // Wait until write is complete

	}

	NVM_On;
	writeSPI2(SST25_WRDI);  // Disables future writings
	NVM_Off;
}


// --- New function writePageNVM -> w25 Page Write -----------
// Write bytes from 1 to 256 bytes (entire page) to w25
// Address must be even, because of aligniment
// little-endian ( LSB first)
void writePageNVM(unsigned char *buf, unsigned short len, unsigned int address)
{
    unsigned short cnt = 0;
    
    // Enable Writes
    NVM_On;
    writeSPI2(SST25_WREN);
    NVM_Off;

    // Send Sequential Write Command
    NVM_On;
    writeSPI2(SST25_WRITE);

    // Send Address to be written
    writeSPI2((address & 0x00FF0000) >> 16);    // MSB
    writeSPI2((address & 0x0000FF00) >> 8);
    writeSPI2((address & 0x000000FF) );    // LSB

    // Send byte or bytes
    do {
        writeSPI2(*(buf + cnt));
        cnt++;
        
    } while (cnt < len);

    NVM_Off;

    // Wait until write is complete    
    while(busyNVM());

    // Disables future writings    
    NVM_On;
    writeSPI2(SST25_WRDI);
    NVM_Off;

}


// --- New function writeBufferNVM -> for w25 Multiple Pages Writes -----------
// Address must be even, because of aligniment
void writeMultiplePageNVM (unsigned char *buf, unsigned short len, unsigned int address)
{
    unsigned short pages = 0;
    unsigned short pages_cnt = 0;
    unsigned short remaining = 0;
    unsigned short bytes_offset = 0;

    pages = len >> 8;

    // send complete pages
    if (pages)
    {
        remaining = len - (pages << 8);
        
        do {
            bytes_offset = (pages_cnt << 8);
            writePageNVM ((buf + bytes_offset), 256, (address + bytes_offset));
            pages_cnt++;
        
        } while (pages_cnt < pages);
    }
    else
        remaining = len;

    // send remaining bytes
    if (remaining)
    {
        bytes_offset = (pages_cnt << 8);
        writePageNVM ((buf + bytes_offset), remaining, (address + bytes_offset));
    }
}


void writeBufferNVM (unsigned char *buff, unsigned short len, unsigned int address)
{
    if (nvm_detected == SST25_DETECTED)
        writeBufNVM16u ((unsigned short *) buff, len >> 1, address);
    else
        writeMultiplePageNVM ((unsigned char *) buff, len, address);

}


