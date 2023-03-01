#ifndef SPI_H
#define SPI_H

#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ReadBufferSize	100
#define WriteBufferSize	100
#define	DontCareByte	(uint8_t)0
#define	CS0_High	SET_BIT(GPIOA->ODR, GPIO_ODR_ODR4)
#define	CS0_Low	CLEAR_BIT(GPIOA->ODR, GPIO_ODR_ODR4)

/**
* @brief SPI State structure definition
*/
typedef enum
{
	SPI_STATE_RESET      = 0x00U,	// Peripheral not Initialized
	SPI_STATE_READY      = 0x01U,	// Peripheral Initialized and ready for use
	SPI_STATE_BUSY       = 0x02U,	// an internal process is ongoing
	SPI_STATE_BUSY_TX    = 0x03U,	// Data Transmission process is ongoing
	SPI_STATE_BUSY_RX    = 0x04U,	// Data Reception process is ongoing
	SPI_STATE_BUSY_TX_RX = 0x05U,	// Data Transmission and Reception process is ongoing
	SPI_STATE_ERROR      = 0x06U,	// SPI error state
	SPI_STATE_ABORT      = 0x07U	// SPI abort is ongoing
}SPI_StateTypeDef;

typedef struct
{
	uint16_t sizeRX;
	uint16_t sizeTX;
	uint16_t countRX;
	uint16_t countTX;
	uint8_t *pBuffRX;
	uint8_t *pBuffTX;
	uint8_t *dataRX;
	uint8_t *dataTX;
	SPI_StateTypeDef state;
}SPI_InstanceTypeDef;

/**
*	@brief Initialize the SPI according to the specified parameters
* @param SPIx: SPI1, SPI2 or SPI3
* @param SPI_Mode: SPI_Mode_Master or SPI_Mode_Slave
* @param SPI_BaudRatePrescaler: SPI_BaudRatePrescaler_2...4,8,16,32,64,128,256
* @retval none
*/
void SPIx_Init(SPI_TypeDef *SPIx, uint16_t SPI_Mode, uint16_t SPI_BaudRatePrescaler);

/**
*	@brief Transmit an amount of data in blocking mode
* @param SPIx: SPI1, SPI2 or SPI3
* @param pBuffer: pointer to data source buffer
* @param lengthTX: No. of bytes to transmit
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus SPI_Transmit(SPI_TypeDef *SPIx, const uint8_t *pBuffer, uint32_t lengthTX);

/**
*	@brief Receive an amount of data in blocking mode
* @param SPIx: SPI1, SPI2 or SPI3
* @param pBuffer: pointer to data destination buffer
* @param lengthTX: No. of bytes to receive
* @retval operation status (SUCCESS or ERROR)
*/
ErrorStatus SPI_Receive(SPI_TypeDef *SPIx, uint8_t *pBuffer, uint32_t lengthRX);

#endif