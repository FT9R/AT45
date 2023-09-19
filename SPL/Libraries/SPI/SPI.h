#ifndef SPI_H
#define SPI_H

#include "Delay.h"
#include <stdlib.h>

#define ReadBufferSize  100
#define WriteBufferSize 100

/**
 * @brief SPI State structure definition
 */
typedef enum {
    SPI_STATE_RESET = 0x00U, // Peripheral not Initialized
    SPI_STATE_READY = 0x01U, // Peripheral Initialized and ready for use
    SPI_STATE_BUSY = 0x02U, // An internal process is ongoing
    SPI_STATE_BUSY_TX = 0x03U, // Data Transmission process is ongoing
    SPI_STATE_BUSY_RX = 0x04U, // Data Reception process is ongoing
    SPI_STATE_BUSY_TX_RX = 0x05U, // Data Transmission and Reception process is ongoing
    SPI_STATE_ERROR = 0x06U, // SPI error state
    SPI_STATE_ABORT = 0x07U // SPI abort is ongoing
} SPI_StateTypeDef;

typedef struct
{
    SPI_TypeDef *Instance;
    uint16_t sizeRX;
    uint16_t sizeTX;
    uint16_t countRX;
    uint16_t countTX;
    uint8_t *pBuffRX;
    uint8_t *pBuffTX;
    SPI_StateTypeDef state;
} SPI_HandleTypeDef;

extern void Error_Handler(void);

/**
 * @brief Initialize the SPI according to the specified parameters
 *
 * @param hspix: pointer to target SPI handle
 * @param SPI_Mode: SPI_Mode_Master or SPI_Mode_Slave
 * @param SPI_BaudRatePrescaler: SPI_BaudRatePrescaler_2...4,8,16,32,64,128,256
 */
void SPIx_Init(SPI_HandleTypeDef *hspix, uint16_t SPI_Mode, uint16_t SPI_BaudRatePrescaler);

/**
 * @brief Transmit an amount of data in blocking mode
 *
 * @param hspix: pointer to target SPI handle
 * @param pData: pointer to data source buffer
 * @param size: number of bytes to transmit
 * @param timeout: timeout duration
 * @return ErrorStatus: SUCCESS or ERROR
 */
ErrorStatus SPI_Transmit(SPI_HandleTypeDef *hspix, const uint8_t *pData, uint16_t size, uint32_t timeout);

/**
 * @brief Receive an amount of data in blocking mode
 *
 * @param hspix: pointer to target SPI handle
 * @param pData: pointer to data destination buffer
 * @param size: number of bytes to receive
 * @param timeout: timeout duration
 * @return ErrorStatus: SUCCESS or ERROR
 */
ErrorStatus SPI_Receive(SPI_HandleTypeDef *hspix, uint8_t *pData, uint16_t size, uint32_t timeout);

#endif