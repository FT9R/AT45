#include "SPI.h"

static ErrorStatus SPI_WaitWithTimeout(SPI_HandleTypeDef *hspix, uint32_t timeout, uint32_t tickStart);
static void SPI_Error_Handler(SPI_HandleTypeDef *hspix);

void SPIx_Init(SPI_HandleTypeDef *hspix, uint16_t SPI_Mode, uint16_t SPI_BaudRatePrescaler)
{
    /* Periphery clock enable */
    switch ((uint32_t) hspix->Instance)
    {
    case (uint32_t) SPI1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        break;

    case (uint32_t) SPI2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        break;

    case (uint32_t) SPI3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
        break;

    default:
        SPI_Error_Handler(hspix); // No match for SPIx
    }

    /* Prepare and reset the handle members */
    hspix->sizeRX = 0;
    hspix->sizeTX = 0;
    hspix->countRX = 0;
    hspix->countTX = 0;
    hspix->pBuffRX = NULL;
    hspix->pBuffTX = NULL;
    hspix->state = SPI_STATE_READY;

    /* Configure the periphery */
    SPI_I2S_DeInit(hspix->Instance);
    SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Hard;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Init(hspix->Instance, &SPI_InitStruct);
    SPI_SSOutputCmd(hspix->Instance, ENABLE);
    SPI_Cmd(hspix->Instance, ENABLE);
}

ErrorStatus SPI_Transmit(SPI_HandleTypeDef *hspix, const uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t tickStart = uwTick;

    if (hspix->state != SPI_STATE_READY)
    {
        SPI_Error_Handler(hspix);
        return ERROR;
    }

    if ((pData == NULL) || (size == 0))
    {
        SPI_Error_Handler(hspix);
        return ERROR;
    }

    hspix->state = SPI_STATE_BUSY_TX;
    hspix->countTX = 0;
    hspix->sizeTX = size;

    while (hspix->countTX < hspix->sizeTX)
    {
        /* TXE bit polling */
        if (SPI_WaitWithTimeout(hspix, timeout, tickStart) != SUCCESS)
        {
            SPI_Error_Handler(hspix);
            return ERROR;
        }
        hspix->Instance->DR = pData[hspix->countTX++];
    }
    hspix->state = SPI_STATE_BUSY;

    /* BSY bit polling */
    if (SPI_WaitWithTimeout(hspix, timeout, tickStart) != SUCCESS)
    {
        SPI_Error_Handler(hspix);
        return ERROR;
    }
    hspix->state = SPI_STATE_READY;

    return SUCCESS;
}

ErrorStatus SPI_Receive(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout)
{
    uint32_t tickStart = uwTick;

    if (hspix->state != SPI_STATE_READY)
    {
        SPI_Error_Handler(hspix);
        return ERROR;
    }

    if ((pData == NULL) || (size == 0))
    {
        SPI_Error_Handler(hspix);
        return ERROR;
    }

    hspix->state = SPI_STATE_BUSY_RX;
    hspix->countRX = 0;
    hspix->sizeRX = size;
    hspix->Instance->DR;

    while (hspix->countRX < hspix->sizeRX)
    {
        hspix->Instance->DR = 0;

        /* RXNE bit polling */
        if (SPI_WaitWithTimeout(hspix, timeout, tickStart) != SUCCESS)
        {
            SPI_Error_Handler(hspix);
            return ERROR;
        }
        pData[hspix->countRX++] = hspix->Instance->DR;
    }
    hspix->state = SPI_STATE_READY;

    return SUCCESS;
}

static ErrorStatus SPI_WaitWithTimeout(SPI_HandleTypeDef *hspix, uint32_t timeout, uint32_t tickStart)
{
    /* Wait for data to be transmitted */
    if (hspix->state == SPI_STATE_BUSY_TX)
    {
        if (timeout == 0) // Infinite timeout
        {
            while (!READ_BIT(hspix->Instance->SR, SPI_SR_TXE)) {}
        }
        else
        {
            while ((uwTick - tickStart) < timeout)
            {
                if (READ_BIT(hspix->Instance->SR, SPI_SR_TXE))
                    return SUCCESS;
            }
        }
        return ERROR;
    }

    /* Wait for data to be received */
    else if (hspix->state == SPI_STATE_BUSY_RX)
    {
        if (timeout == 0) // Infinite timeout
        {
            while (!READ_BIT(hspix->Instance->SR, SPI_SR_RXNE)) {}
        }
        else
        {
            while ((uwTick - tickStart) < timeout)
            {
                if (READ_BIT(hspix->Instance->SR, SPI_SR_RXNE))
                    return SUCCESS;
            }
        }
        return ERROR;
    }

    /* Wait for shift register to be empty */
    else if (hspix->state == SPI_STATE_BUSY)
    {
        if (timeout == 0)
        {
            while (READ_BIT(hspix->Instance->SR, SPI_SR_BSY)) {}
        }
        else
        {
            while ((uwTick - tickStart) < timeout)
            {
                if (!READ_BIT(hspix->Instance->SR, SPI_SR_BSY))
                    return SUCCESS;
            }
        }
        return ERROR;
    }

    /* Unexpected state */
    return ERROR;
}

static void SPI_Error_Handler(SPI_HandleTypeDef *hspix)
{
    hspix->state = SPI_STATE_ERROR;

    /* FIXME: исключить после отладки */
    Error_Handler();
}

/* Under construction */
// ErrorStatus SPI_Transmit_IT(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX)
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
// }

// uint32_t SPI_Receive_IT(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX)
//{
//	if (SPIx == SPI1)
//	{
//		if (SPI1_Instance.dataRX.status == Idle)
//		{
//			SPI1->DR;
//			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
//			SPI1_Instance.dataRX.status = Processing;
//			SPI1_Instance.dataRX.pBuffer = pBuffer;
//			SPI1_Instance.dataRX.length = lengthRX;
//			SPI1->DR = DontCareByte;
//		}
//	}
//	return 0;
// }

////#ifdef SPI1_Enable
// void SPI1_IRQHandler(void)
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
// #ifndef WipeBufferEachRead
//				memcpy(SPI1_Instance.dataRX.pBuffer, SPI1_Instance.dataRX.data,
// SPI1_Instance.dataRX.length); #else 				memcpy(SPI1_Instance.dataRX.pBuffer,
// SPI1_Instance.dataRX.data,
// SPI1_Instance.dataRX.counter); #endif 				SPI1_Instance.dataRX.counter = 0;
// SPI1_Instance.dataRX.status = Idle;
//			}
//		}
//	}
// }
////#endif