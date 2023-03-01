#include "main.h"

const uint8_t bufferWrite[] =
{1, 2, 3, 4, 5, 6, 7, 8, 9, 0,\
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'};
uint8_t bufferRead[sizeof(bufferWrite)];
extern uint8_t AT45_ID[5];
bool FLASH_access;


void main()
{
	asm("CPSID i");
	Sys_Init();
	IO_Init();
	SPIx_Init(SPI1, SPI_Mode_Master, SPI_BaudRatePrescaler_256);
	asm("CPSIE i");
	_delay_ms(200);

	/* Check if FLASH device is accessible */
	AT45_ID_Read(SPI1, AT45_ID);
	if (AT45_ID[0] == ManufacturerID_DEF) FLASH_access = true;
	else FLASH_access = false;

	if (FLASH_access)
	{
		memset(bufferRead, NULL, sizeof(bufferRead));
		printf("\r\n First approach to read \r\n");
		AT45_Read(SPI1, bufferRead, sizeof(bufferRead), PAGE_TO_WRITE);
		if (strncmp((const char*)bufferRead, (const char*)bufferWrite, sizeof(bufferRead)) == 0)
		{
			printf("Data already exist at page %i boundaries \r\n", PAGE_TO_WRITE);
		}
		else
		{
			printf("Data doesn't exist at page %i boundaries \r\n", PAGE_TO_WRITE);
			printf("Page programming...\r\n");
			AT45_Write(SPI1, buffer_1, bufferWrite, sizeof(bufferWrite), PAGE_TO_WRITE);
			printf("\r\n Second approach to read \r\n");
			AT45_Read(SPI1, bufferRead, sizeof(bufferRead), PAGE_TO_WRITE);
			if (strncmp((const char*)bufferRead, (const char*)bufferWrite, sizeof(bufferRead)) == 0)
			{
				printf("Writing process success \r\n");
			}
			else printf("Writing process failure \r\n");
		}
	}
	else printf("Couldn't get any response from device \r\n");

	while(1);
}