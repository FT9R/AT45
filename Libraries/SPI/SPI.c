#include "SPI.h"

SPI_InitTypeDef SPI_InitStruct;
SPI_InstanceTypeDef SPI1_Instance;
SPI_InstanceTypeDef SPI2_Instance;
SPI_InstanceTypeDef SPI3_Instance;


void SPIx_Init(SPI_TypeDef *SPIx, uint16_t SPI_Mode, uint16_t SPI_BaudRatePrescaler)
{
	switch ((uint32_t)SPIx)
	{
	case (uint32_t)SPI1:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
		SPI1_Instance.countRX = 0;
		SPI1_Instance.countTX = 0;
		SPI1_Instance.dataRX = (uint8_t*) calloc(ReadBufferSize, sizeof(uint8_t));
		SPI1_Instance.dataRX = (uint8_t*) calloc(WriteBufferSize, sizeof(uint8_t));
		SPI1_Instance.pBuffRX = NULL;
		SPI1_Instance.pBuffTX = NULL;
		SPI1_Instance.sizeRX = 0;
		SPI1_Instance.sizeTX = 0;
		SPI1_Instance.state = SPI_STATE_READY;
		NVIC_EnableIRQ(SPI1_IRQn);
		break;
	case (uint32_t)SPI2:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
		SPI2_Instance.countRX = 0;
		SPI2_Instance.countTX = 0;
		SPI2_Instance.dataRX = (uint8_t*) calloc(ReadBufferSize, sizeof(uint8_t));
		SPI2_Instance.dataRX = (uint8_t*) calloc(WriteBufferSize, sizeof(uint8_t));
		SPI2_Instance.pBuffRX = NULL;
		SPI2_Instance.pBuffTX = NULL;
		SPI2_Instance.sizeRX = 0;
		SPI2_Instance.sizeTX = 0;
		SPI2_Instance.state = SPI_STATE_READY;;
		NVIC_EnableIRQ(SPI2_IRQn);
		break;
	case (uint32_t)SPI3:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
		SPI3_Instance.countRX = 0;
		SPI3_Instance.countTX = 0;
		SPI3_Instance.dataRX = (uint8_t*) calloc(ReadBufferSize, sizeof(uint8_t));
		SPI3_Instance.dataRX = (uint8_t*) calloc(WriteBufferSize, sizeof(uint8_t));
		SPI3_Instance.pBuffRX = NULL;
		SPI3_Instance.pBuffTX = NULL;
		SPI3_Instance.sizeRX = 0;
		SPI3_Instance.sizeTX = 0;
		SPI3_Instance.state = SPI_STATE_READY;
		NVIC_EnableIRQ(SPI3_IRQn);
		break;
	}

	SPI_I2S_DeInit(SPIx);
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Hard;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPIx, &SPI_InitStruct);
	SPI_SSOutputCmd(SPIx, ENABLE);
	SPI_Cmd(SPIx, ENABLE);
}

ErrorStatus SPI_Transmit(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX)
{
	if (SPIx == SPI1)
	{
		while (SPI1_Instance.state != SPI_STATE_READY);
		SPI1_Instance.state = SPI_STATE_BUSY_TX;
		SPI1_Instance.countTX = 0;
		SPI1_Instance.sizeTX = lengthTX;
		while (SPI1_Instance.countTX < lengthTX)
		{
			while (!READ_BIT(SPI1->SR, SPI_SR_TXE));
			SPI1->DR = pBuffer[SPI1_Instance.countTX++];
		}
		while (READ_BIT(SPI1->SR, SPI_SR_BSY));
		SPI1_Instance.state = SPI_STATE_READY;
		return SUCCESS;
	}
	else if (SPIx == SPI2)
	{
		while (SPI2_Instance.state != SPI_STATE_READY);
		SPI2_Instance.state = SPI_STATE_BUSY_TX;
		SPI2_Instance.countTX = 0;
		SPI2_Instance.sizeTX = lengthTX;
		while (SPI2_Instance.countTX < lengthTX)
		{
			while (!READ_BIT(SPI2->SR, SPI_SR_TXE));
			SPI2->DR = pBuffer[SPI2_Instance.countTX++];
		}
		while (READ_BIT(SPI2->SR, SPI_SR_BSY));
		SPI2_Instance.state = SPI_STATE_READY;
		return SUCCESS;
	}
	else if (SPIx == SPI3)
	{
		while (SPI3_Instance.state != SPI_STATE_READY);
		SPI3_Instance.state = SPI_STATE_BUSY_TX;
		SPI3_Instance.countTX = 0;
		SPI3_Instance.sizeTX = lengthTX;
		while (SPI3_Instance.countTX < lengthTX)
		{
			while (!READ_BIT(SPI3->SR, SPI_SR_TXE));
			SPI3->DR = pBuffer[SPI3_Instance.countTX++];
		}
		while (READ_BIT(SPI3->SR, SPI_SR_BSY));
		SPI3_Instance.state = SPI_STATE_READY;
		return SUCCESS;
	}
	return ERROR; // no match SPIx
}

ErrorStatus SPI_Receive(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX)
{
	if (SPIx == SPI1)
	{
		while (SPI1_Instance.state != SPI_STATE_READY);
		SPI1_Instance.state = SPI_STATE_BUSY_RX;
		SPI1_Instance.countRX = 0;
		SPI1_Instance.sizeRX = lengthRX;
		(void) SPI1->DR;
		while (SPI1_Instance.countRX < lengthRX)
		{
			SPI1->DR = DontCareByte;
			while (!READ_BIT(SPI1->SR, SPI_SR_RXNE));
			pBuffer[SPI1_Instance.countRX++] = SPI1->DR;
		}
		SPI1_Instance.state = SPI_STATE_READY;
		return SUCCESS;
	}
	else if (SPIx == SPI2)
	{
		while (SPI2_Instance.state != SPI_STATE_READY);
		SPI2_Instance.state = SPI_STATE_BUSY_RX;
		SPI2_Instance.countRX = 0;
		SPI2_Instance.sizeRX = lengthRX;
		(void) SPI2->DR;
		while (SPI2_Instance.countRX < lengthRX)
		{
			SPI2->DR = DontCareByte;
			while (!READ_BIT(SPI2->SR, SPI_SR_RXNE));
			pBuffer[SPI2_Instance.countRX++] = SPI2->DR;
		}
		SPI2_Instance.state = SPI_STATE_READY;
		return SUCCESS;
	}
	else if (SPIx == SPI3)
	{
		while (SPI3_Instance.state != SPI_STATE_READY);
		SPI3_Instance.state = SPI_STATE_BUSY_RX;
		SPI3_Instance.countRX = 0;
		SPI3_Instance.sizeRX = lengthRX;
		(void) SPI3->DR;
		while (SPI3_Instance.countRX < lengthRX)
		{
			SPI3->DR = DontCareByte;
			while (!READ_BIT(SPI3->SR, SPI_SR_RXNE));
			pBuffer[SPI3_Instance.countRX++] = SPI3->DR;
		}
		SPI3_Instance.state = SPI_STATE_READY;
		return SUCCESS;
	}
	return ERROR; // no match SPIx
}


/* Under construction */
//ErrorStatus SPI_Transmit_IT(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX)
//{
//	if (SPIx == SPI1)
//	{
//		if (SPI1_Instance.dataTX.status == Idle)
//		{
//			SPI1_Instance.dataTX.status = Processing;
//			SPI1_Instance.dataTX.counter = 0;
//			SPI1_Instance.dataTX.length = lengthTX;
//			if (lengthTX <= WriteBufferSize)
//			{
//				memcpy(SPI1_Instance.dataTX.data, pBuffer, lengthTX);
//				SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, ENABLE);
//			}
//			else return ERROR;
//		}
//		else
//		{
//			if ((SPI1_Instance.dataTX.length + lengthTX) < WriteBufferSize)
//			{
//				memcpy((SPI1_Instance.dataTX.data + SPI1_Instance.dataTX.length), pBuffer, lengthTX);
//				SPI1_Instance.dataTX.length += lengthTX;
//			}
//			else return ERROR;
//		}
//	}
//	return SUCCESS;
//}

//uint32_t SPI_Receive_IT(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX)
//{
//	if (SPIx == SPI1)
//	{
//		if (SPI1_Instance.dataRX.status == Idle)
//		{
//			(void) SPI1->DR;
//			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
//			SPI1_Instance.dataRX.status = Processing;
//			SPI1_Instance.dataRX.pBuffer = pBuffer;
//			SPI1_Instance.dataRX.length = lengthRX;
//			SPI1->DR = DontCareByte;
//		}
//	}
//	return 0;
//}

////#ifdef SPI1_Enable
//void SPI1_IRQHandler(void)
//{
//	if (READ_BIT(SPI1->SR, SPI_SR_TXE))
//	{
//		if (SPI1_Instance.dataTX.status == Processing)
//		{
//			if (SPI1_Instance.dataTX.counter < SPI1_Instance.dataTX.length)
//			{
//				SPI1->DR = SPI1_Instance.dataTX.data[SPI1_Instance.dataTX.counter++];
//			}
//			else if (!READ_BIT(SPI1->SR, SPI_SR_BSY))
//			{
//				SPI1_Instance.dataTX.status = Idle;
//				SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_TXE, DISABLE);
//			}
//		}
//	}
//	if (READ_BIT(SPI1->SR, SPI_SR_RXNE))
//	{
//		if (SPI1_Instance.dataRX.status == Processing)
//		{
//			SPI1_Instance.dataRX.data[SPI1_Instance.dataRX.counter++] = (uint8_t)SPI1->DR;
//			if (SPI1_Instance.dataRX.counter < SPI1_Instance.dataRX.length) SPI1->DR = DontCareByte;
//			else
//			{
//				SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, DISABLE);
//#ifndef WipeBufferEachRead
//				memcpy(SPI1_Instance.dataRX.pBuffer, SPI1_Instance.dataRX.data, SPI1_Instance.dataRX.length);
//#else
//				memcpy(SPI1_Instance.dataRX.pBuffer, SPI1_Instance.dataRX.data, SPI1_Instance.dataRX.counter);
//#endif
//				SPI1_Instance.dataRX.counter = 0;
//				SPI1_Instance.dataRX.status = Idle;
//			}
//		}
//	}
//}
////#endif