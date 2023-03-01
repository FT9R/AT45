#ifndef	AT45_H
#define	AT45_H

#include "SPI.h"
#include "Delay.h"
#include "stdbool.h"

#define ManufacturerID_DEF	0x1F
#define MainMemoryPageRead_DEF 0xD2
#define MainMemoryPageProgramThroughBuffer1_DEF 0x82
#define MainMemoryPageProgramThroughBuffer2_DEF 0x85
#define StatusRegisterRead_DEF 0xD7
#define PageErase_DEF 0x81
#define DeviceID_DEF 0x9F

/* Timings(AT45DB161E) */
#define PageEraseTime	35	// ms
#define PageEraseAndProgrrammingTime	25	// ms
#define ChipEraseTime	40000	// ms

typedef enum
{
	buffer_1 = 1,
	buffer_2
}buffer_TypeDef;

/**
* @brief  Writes a Data from extern buffer to AT45 through the SPIx
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param	AT_bufx: describes which buffer of AT45 is used to send data from bus to main memory
* @param  Buf: pointer to extern buffer, that contains the data to send
* @param	BufSize: size of extern buffer, that contains the data to send
* @param	PA_write: page address to write[0-4095]
*	@note		current function writes data to desired page from zero position and automatically increase the BFA
* @note		this is a long time process, so try to use it carefully relatively to processor time
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus AT45_Write(SPI_TypeDef *SPIx, buffer_TypeDef AT_bufx, const uint8_t *Buf, uint16_t BufSize, uint16_t PA_write);

/**
* @brief  Reades a Data from AT45 to extern buffer through the SPIx
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param  Buf: pointer to extern buffer, that will contain data received from AT45
* @param	BufSize: size of extern buffer, that will contain data received from AT45
* @param	PA_read: page address to read
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus AT45_Read(SPI_TypeDef *SPIx, uint8_t *Buf, uint16_t BufSize, uint16_t PA_read);

/**
* @brief  Reades a Manufacturer and Device ID
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @param	ID_Buf: pointer to extern buffer, that will contain 5 bytes of information about device
* @retval None
*/
void AT45_ID_Read(SPI_TypeDef *SPIx, uint8_t *ID_Buf);

/**
* @brief  Checks if IC is busy or ready
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
* @retval true in case if IC is busy and vice versa
*/
bool IS_Busy(SPI_TypeDef *SPIx);

/**
* @brief  Involves full chip erase
* @param  SPIx: where x can be 1, 2 or 3 in SPI mode
*	@note		In a certain percentage of units, the Chip Erase feature may not function correctly and may adversely affect device operation
* @retval None
*/
void AT45_Chip_Erase(SPI_TypeDef *SPIx);

#endif