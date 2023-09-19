#ifndef AT45_H
#define AT45_H

#include "AT45_Interface.h"

/* Instruction Set */
#define AT45_CMD_MAIN_MEMORY_PAGE_READ                           0xD2
#define AT45_CMD_BUFFER_1_WRITE                                  0x84
#define AT45_CMD_BUFFER_2_WRITE                                  0x87
#define AT45_CMD_BUFFER_1_TO_MAIN_MEMORY_PAGE_PROGRAM_ERASE      0x83
#define AT45_CMD_BUFFER_2_TO_MAIN_MEMORY_PAGE_PROGRAM_ERASE      0x86
#define AT45_CMD_BUFFER_1_TO_MAIN_MEMORY_PAGE_PROGRAM            0x88
#define AT45_CMD_BUFFER_2_TO_MAIN_MEMORY_PAGE_PROGRAM            0x89
#define AT45_CMD_MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER_1_ERASE 0x82
#define AT45_CMD_MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER_2_ERASE 0x85
#define AT45_CMD_MAIN_MEMORY_PAGE_PROGRAM_THROUGH_BUFFER_1       0x02
#define AT45_CMD_PAGE_ERASE                                      0x81
#define AT45_CMD_BLOCK_ERASE                                     0x50
#define AT45_CMD_SECTOR_ERASE                                    0x7C
#define AT45_CMD_CHIP_ERASE_0                                    0xC7
#define AT45_CMD_CHIP_ERASE_1                                    0x94
#define AT45_CMD_CHIP_ERASE_2                                    0x80
#define AT45_CMD_CHIP_ERASE_3                                    0x9A
#define AT45_CMD_STATUS_REGISTER_READ                            0xD7
#define AT45_CMD_MANUFACTURER_DEVICE_ID_READ                     0x9F
#define AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_0                    0x3D
#define AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_1                    0x2A
#define AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_2                    0x80
#define AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_3                    0xA6
#define AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_0                  0x3D
#define AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_1                  0x2A
#define AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_2                  0x80
#define AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_3                  0xA7

/* Timings [ms] */
#define AT45_PAGE_ERASE_PROGRAMMING_TIME 25
#define AT45_PAGE_PROGRAMMING_TIME       4
#define AT45_PAGE_ERASE_TIME             35
#define AT45_BLOCK_ERASE_TIME            100
#define AT45_SECTOR_ERASE_TIME           2000
#define AT45_CHIP_ERASE_TIME             40000

/* Timeouts [ms] */
#define AT45_TX_TIMEOUT       100
#define AT45_RX_TIMEOUT       100
#define AT45_RESPONSE_TIMEOUT 100

/* Device constants */
#define AT45_MANUFACTURER_ID 0x1F
#define AT45_PAGE_SIZE       512
typedef enum { AT45DB021 = 0x23, AT45DB041, AT45DB081, AT45DB161, AT45DB321, AT45DB641 } AT45_t;

/* Macro */
#define KB_TO_BYTE(KB)         (KB * 1024)
#define CS_HIGH(DEVICE_HANDLE) SET_BIT((DEVICE_HANDLE)->CS_Port->BSRR, (DEVICE_HANDLE)->CS_Pin)
#define CS_LOW(DEVICE_HANDLE)  SET_BIT((DEVICE_HANDLE)->CS_Port->BSRR, (DEVICE_HANDLE)->CS_Pin << 16)
#define ADDRESS_BYTES_SWAP(DEVICE_HANDLE, ADDRESS)              \
    DEVICE_HANDLE->addressBytes[0] = (uint8_t) (ADDRESS >> 16); \
    DEVICE_HANDLE->addressBytes[1] = (uint8_t) (ADDRESS >> 8);  \
    DEVICE_HANDLE->addressBytes[2] = (uint8_t) (ADDRESS >> 0)

typedef enum { AT45_PAGE_ERASE, AT45_BLOCK_ERASE, AT45_SECTOR_ERASE, AT45_CHIP_ERASE } eraseInstruction_t;

typedef enum { AT45_WAIT_NO, AT45_WAIT_DELAY, AT45_WAIT_BUSY } waitForTask_t;

typedef struct
{
    uint8_t ID[5];
    uint8_t statusRegister[2];
    uint8_t addressBytes[3];
    uint8_t CMD[4];
    SPI_HandleTypeDef *hspix;
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
    uint32_t numberOfPages;
    ErrorStatus status;
} AT45_HandleTypeDef;

/**
 * @brief Checks if the device is available and determines the number of pages
 * @param AT45_Handle: pointer to the device handle structure
 * @param hspix: pointer to target SPI handle
 * @param CS_Port: GPIOx
 * @param CS_Pin: GPIO_Pin_x
 * @return Operation status
 */
ErrorStatus AT45_Init(AT45_HandleTypeDef *AT45_Handle, SPI_HandleTypeDef *hspix, GPIO_TypeDef *CS_Port,
                      uint16_t CS_Pin);

/**
 * @brief Writes data to AT45 from external buffer
 * @param AT45_Handle: pointer to the device handle structure
 * @param  buf: pointer to external buffer, that contains the data to write
 * @param dataLength: number of bytes to write
 * @param address: page address to write (multiple of 512 bytes)
 * @param trailingCRC: insert or not insert CRC at the end of frame
 * @param pageErase: erase or not erase page before the write operation
 * @param waitForTask: the way to ensure that operation is completed
 * @return Operation status
 */
ErrorStatus AT45_Write(AT45_HandleTypeDef *AT45_Handle, const uint8_t *buf, uint16_t dataLength, uint32_t address,
                       bool trailingCRC, bool pageErase, waitForTask_t waitForTask);

/**
 * @brief Reades data from AT45 to external buffer
 * @param AT45_Handle: pointer to the device handle structure
 * @param  buf: pointer to external buffer, that will contain the received data
 * @param dataLength: number of bytes to read
 * @param address: page address to read (multiple of 512 bytes)
 * @param trailingCRC: compare or not compare CRC at the end of frame
 * @return Operation status
 */
ErrorStatus AT45_Read(AT45_HandleTypeDef *AT45_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                      bool trailingCRC);

/**
 * @brief Begins erase operation of page, block, sector or whole memory array
 * @param AT45_Handle: pointer to the device handle structure
 * @param eraseInstruction: pages groups to be erased
 * @param address: start address of page, block or sector to be erased
 * @param waitForTask: the way to ensure that operation is completed
 * @return Operation status
 * @note Address has to be NULL in case of chip erase
 */
ErrorStatus AT45_Erase(AT45_HandleTypeDef *AT45_Handle, eraseInstruction_t eraseInstruction, uint32_t address,
                       waitForTask_t waitForTask);

/**
 * @brief Checks if the device is busy or not
 * @param AT45_Handle: pointer to the device handle structure
 * @return True - device is busy
 */
bool AT45_Busy(AT45_HandleTypeDef *AT45_Handle);

#endif