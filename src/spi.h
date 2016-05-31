#ifndef SPI_H
#define SPI_H

#define SPI_RXNE	0x0001
#define SPI_TXE		0x0002
#define SPI_BSY		0x0080
#define SPI_OVR		0x0040

//-------- Functions -------------
void SPI_Config(void);
void Send_SPI (unsigned char * , unsigned char);
unsigned char Send_Receive_SPI (unsigned char a);
void Wait_SPI_Busy (void);
void Send_SPI_Multiple (unsigned char);
void Send_SPI_Single (unsigned char);
unsigned char Receive_SPI_Single (void);

#endif
