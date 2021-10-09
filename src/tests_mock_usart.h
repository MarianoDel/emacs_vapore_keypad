//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS_MOCK_USART.H ####################
//---------------------------------------------

// Module Exported Functions ---------------------------------------------------
void Usart1Send (char * msg);
unsigned char Usart1ReadBuffer (unsigned char * bout, unsigned short max_len);
void Usart1FillRxBuffer (char * msg);
void Usart1OuputEnable (unsigned char ena);

void Usart2Send (char * msg);
unsigned char Usart2ReadBuffer (unsigned char * bout, unsigned short max_len);
void Usart2FillRxBuffer (char * msg);


//--- end of file ---//


