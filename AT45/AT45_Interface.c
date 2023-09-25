#include "AT45_Interface.h"

void AT45_SPI_Transmit(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout)
{
#ifdef USE_HAL_DRIVER
    HAL_SPI_Transmit(hspix, pData, size, timeout);
#else
    if (SPI_Transmit(hspix, pData, size, timeout) != SPI_STATE_READY)
        Error_Handler();
#endif
}

void AT45_SPI_Receive(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout)
{
#ifdef USE_HAL_DRIVER
    HAL_SPI_Receive(hspix, pData, size, timeout);
#else
    if (SPI_Receive(hspix, pData, size, timeout) != SPI_STATE_READY)
        Error_Handler();
#endif
}

void AT45_Delay(uint32_t ms)
{
#ifdef USE_HAL_DRIVER
    HAL_Delay(ms);
#else
    Delay(ms);
#endif
}