#ifndef AT45_INTERFACE_H
#define AT45_INTERFACE_H

#include "SPI.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void AT45_SPI_Transmit(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout);
void AT45_SPI_Receive(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout);
void AT45_Delay(uint32_t ms);

#endif