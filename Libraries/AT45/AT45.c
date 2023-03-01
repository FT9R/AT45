#include "AT45.h"

const uint8_t ChipEraseSequence_CMD[4] = {0xC7, 0x94, 0x80, 0x9A};
uint8_t AT45_ID[5];
const uint8_t MainMemoryPageRead_CMD = MainMemoryPageRead_DEF;
const uint8_t MainMemoryPageProgramThroughBuffer1_CMD = MainMemoryPageProgramThroughBuffer1_DEF;
const uint8_t MainMemoryPageProgramThroughBuffer2_CMD = MainMemoryPageProgramThroughBuffer2_DEF;
const uint8_t StatusRegisterRead_CMD = StatusRegisterRead_DEF;
const uint8_t PageErase_CMD = PageErase_DEF;
const uint8_t DeviceID_CMD = DeviceID_DEF;
const uint8_t DummyByte = NULL;


ErrorStatus AT45_Write(SPI_TypeDef *SPIx, buffer_TypeDef AT_bufx, const uint8_t *Buf, uint16_t BufSize, uint16_t PA_write)
{
	if ((BufSize > 527) || (PA_write > 4095)) return ERROR;
	if (IS_Busy(SPIx)) return ERROR;
	uint32_t AddressBytes = 0;

	CS0_High;
	CS0_Low;
	for (uint16_t BFA = 0; BFA < BufSize; BFA++)	// BFA designates a byte address within a buffer
	{
		switch (AT_bufx)
		{
		case buffer_1:
			SPI_Transmit(SPIx, &MainMemoryPageProgramThroughBuffer1_CMD, 1);
			break;
		case buffer_2:
			SPI_Transmit(SPIx, &MainMemoryPageProgramThroughBuffer2_CMD, 1);
			break;
		}
		/* Forming the sequence of address bytes */
		AddressBytes = (BFA & 0x3FF);	// 10 BFA bits
		AddressBytes |= (PA_write & 0xFFF) << 10;	// 12 PA bits
		/* Clock three address bytes into AT45 */
		const uint8_t threeAddrBytes[3] = {(uint8_t)(AddressBytes >> 16), (uint8_t)(AddressBytes >> 8), (uint8_t)AddressBytes};
		SPI_Transmit(SPI1, threeAddrBytes, 3);
		/* Data write */
		SPI_Transmit(SPIx, &Buf[BFA], 1);
	}
	CS0_High;
	_delay_ms(PageEraseAndProgrrammingTime);
	return SUCCESS;
}

ErrorStatus AT45_Read(SPI_TypeDef *SPIx, uint8_t *Buf, uint16_t BufSize, uint16_t PA_read)
{
	if ((BufSize > 527) || (PA_read > 4095)) return ERROR;
	if (IS_Busy(SPIx)) return ERROR;
	bool FirstRead = true;
	uint32_t AddressBytes = 0;

	CS0_High;
	CS0_Low;
	for (uint16_t BA = 0; BA < BufSize; BA++) // BA designates a byte address within the page
	{
		SPI_Transmit(SPIx, &MainMemoryPageRead_CMD, 1);
		/* Forming the sequence of address bytes */
		AddressBytes = (BA & 0x3FF);	// 10 BA bits
		AddressBytes |= (PA_read & 0xFFF) << 10;	// 12 PA bits
		/* Clock three address bytes into AT45 */
		const uint8_t threeAddrBytes[3] = {(uint8_t)(AddressBytes >> 16), (uint8_t)(AddressBytes >> 8), (uint8_t)AddressBytes};
		SPI_Transmit(SPI1, threeAddrBytes, 3);
		/* 4 don't care bytes to initialize the read operation */
		if (FirstRead)
		{
			SPI_Transmit(SPI1, &DummyByte, 1);
			SPI_Transmit(SPI1, &DummyByte, 1);
			SPI_Transmit(SPI1, &DummyByte, 1);
			SPI_Transmit(SPI1, &DummyByte, 1);
			FirstRead = false;
		}
		SPI_Receive(SPI1, &Buf[BA], 1);
	}
	CS0_High;
	return SUCCESS;
}

void AT45_ID_Read(SPI_TypeDef *SPIx, uint8_t *ID_Buf)
{
	if (IS_Busy(SPIx)) return;
	CS0_High;
	CS0_Low;
	SPI_Transmit(SPIx, &DeviceID_CMD, 1);
	SPI_Receive(SPIx, ID_Buf, 5);
	CS0_High;
}

#pragma optimize = no_size_constraints no_inline
bool IS_Busy(SPI_TypeDef *SPIx)
{
	CS0_High;
	CS0_Low;
	SPI_Transmit(SPIx, &StatusRegisterRead_CMD, 1);
	uint16_t Timeout = 0;
	uint8_t SRR_Byte = 0;
	while ((SRR_Byte & (1<<7)) == false)
	{
		if ((Timeout++) > 60000) {CS0_High; return true;}
		SPI_Receive(SPIx, &SRR_Byte, 1);
	}
	CS0_High;
	return false;
}

void AT45_Chip_Erase(SPI_TypeDef *SPIx)
{
	if (IS_Busy(SPIx)) return;
	CS0_High;
	CS0_Low;
	for (int i = 0; i < sizeof(ChipEraseSequence_CMD); i++)
	{
		SPI_Transmit(SPIx, &ChipEraseSequence_CMD[i], 1);
	}
	CS0_High;
	for (uint16_t delCnt = 0; delCnt < (ChipEraseTime/200); delCnt++) _delay_ms(200);
}