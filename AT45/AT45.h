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
#define AT45_PAGE_PROGRAMMING_TIME       4
#define AT45_PAGE_ERASE_PROGRAMMING_TIME 25
#define AT45_PAGE_ERASE_TIME             35
#define AT45_BLOCK_ERASE_TIME            100
#define AT45_SECTOR_ERASE_TIME           2000
#define AT45_CHIP_ERASE_TIME             40000

/* Timeouts [ms] */
#define AT45_TX_TIMEOUT       100
#define AT45_RX_TIMEOUT       100
#define AT45_RESPONSE_TIMEOUT 100

/* Device constants */
#define AT45_MANUFACTURER_ID    0x1F

#define AT45DB021E_PAGE_SIZE_BINARY     256
#define AT45DB021E_PAGE_SIZE_STANDARD   264
#define AT45DB041E_PAGE_SIZE_BINARY     256
#define AT45DB041E_PAGE_SIZE_STANDARD   264
#define AT45DB081E_PAGE_SIZE_BINARY     256
#define AT45DB081E_PAGE_SIZE_STANDARD   264
#define AT45DB161E_PAGE_SIZE_BINARY     512
#define AT45DB161E_PAGE_SIZE_STANDARD   528
#define AT45DB321E_PAGE_SIZE_BINARY     512
#define AT45DB321E_PAGE_SIZE_STANDARD   528
#define AT45DB641E_PAGE_SIZE_BINARY     256
#define AT45DB641E_PAGE_SIZE_STANDARD   264

#define AT45DB021E_BLOCK_SIZE_BINARY     (AT45DB021E_PAGE_SIZE_BINARY*8)
#define AT45DB021E_BLOCK_SIZE_STANDARD   (AT45DB021E_PAGE_SIZE_STANDARD*8)
#define AT45DB041E_BLOCK_SIZE_BINARY     (AT45DB041E_PAGE_SIZE_BINARY*8)
#define AT45DB041E_BLOCK_SIZE_STANDARD   (AT45DB041E_PAGE_SIZE_STANDARD*8)
#define AT45DB081E_BLOCK_SIZE_BINARY     (AT45DB081E_PAGE_SIZE_BINARY*8)
#define AT45DB081E_BLOCK_SIZE_STANDARD   (AT45DB081E_PAGE_SIZE_STANDARD*8)
#define AT45DB161E_BLOCK_SIZE_BINARY     (AT45DB161E_PAGE_SIZE_BINARY*8)
#define AT45DB161E_BLOCK_SIZE_STANDARD   (AT45DB161E_PAGE_SIZE_STANDARD*8)
#define AT45DB321E_BLOCK_SIZE_BINARY     (AT45DB321E_PAGE_SIZE_BINARY*8)
#define AT45DB321E_BLOCK_SIZE_STANDARD   (AT45DB321E_PAGE_SIZE_STANDARD*8)
#define AT45DB641E_BLOCK_SIZE_BINARY     (AT45DB641E_PAGE_SIZE_BINARY*8)
#define AT45DB641E_BLOCK_SIZE_STANDARD   (AT45DB641E_PAGE_SIZE_STANDARD*8)

enum AT45_DeviceID_e { AT45DB021 = 0x23, AT45DB041, AT45DB081, AT45DB161, AT45DB321, AT45DB641 };
enum AT45_PageSizeOptions_e {AT45_PAGE_SIZE_BINARY , AT45_PAGE_SIZE_ACTUAL };

/* Macro */
#define KB_TO_BYTE(KB)         ((KB) * 1024)
#define CS_HIGH(DEVICE_HANDLE) SET_BIT((DEVICE_HANDLE)->CS_Port->BSRR, (DEVICE_HANDLE)->CS_Pin)
#define CS_LOW(DEVICE_HANDLE)  SET_BIT((DEVICE_HANDLE)->CS_Port->BSRR, (DEVICE_HANDLE)->CS_Pin << 16)

/* Data types */
typedef enum AT45_EraseInstruction_e {
    AT45_PAGE_ERASE,
    AT45_BLOCK_ERASE,
    AT45_SECTOR_ERASE,
    AT45_CHIP_ERASE
} AT45_EraseInstruction_t;

typedef enum AT45_WaitForTask_e { AT45_WAIT_NO, AT45_WAIT_DELAY, AT45_WAIT_BUSY } AT45_WaitForTask_t;

typedef enum AT45_Status_e {
    AT45_STATUS_RESET,
    AT45_STATUS_READY,
    AT45_STATUS_BUSY_WRITE,
    AT45_STATUS_BUSY_READ,
    AT45_STATUS_BUSY_ERASE,
    AT45_STATUS_ERROR_INITIALIZATION,
    AT45_STATUS_ERROR_ARGUMENT,
    AT45_STATUS_ERROR_TIMEOUT,
    AT45_STATUS_ERROR_MEM_MANAGE,
    AT45_STATUS_ERROR_CHECKSUM,
    AT45_STATUS_ERROR_INSTRUCTION
} AT45_Status_t;

typedef struct AT45_HandleTypeDef_s
{
    SPI_HandleTypeDef *hspix;
    GPIO_TypeDef *CS_Port;
    uint16_t CS_Pin;
    uint8_t ID[5];
    uint8_t statusRegister[2];
    uint8_t addressBytes[3];
    uint8_t CMD[4];
    uint32_t numberOfPages;
    uint32_t pageSize;
    AT45_Status_t status;
} AT45_HandleTypeDef;

/**
 * @brief Checks if the device is available and determines the number of pages
 * @param AT45_Handle: pointer to the device handle structure
 * @param hspix: pointer to target SPI handle
 * @param CS_Port: GPIOx
 * @param CS_Pin: GPIO_Pin_x
 * @return Device status
 */
AT45_Status_t AT45_Init(
    AT45_HandleTypeDef *AT45_Handle, 
    SPI_HandleTypeDef *hspix, 
    GPIO_TypeDef *CS_Port,
    uint16_t CS_Pin
);

/**
 * @brief Writes data to ROM from external buffer
 * @param AT45_Handle: pointer to the device handle structure
 * @param buf: pointer to external buffer, that contains the data to write
 * @param dataLength: number of bytes to write
 * @param address: page address to write (multiple of 512 bytes)
 * @param trailingCRC: insert or not insert CRC at the end of frame
 * @param pageErase: erase or not erase page before the write operation
 * @param waitForTask: the way to ensure that operation is completed
 * @return Device status
 */
AT45_Status_t AT45_Write(AT45_HandleTypeDef *AT45_Handle, const uint8_t *buf, uint16_t dataLength, uint32_t address,
                         bool trailingCRC, bool pageErase, AT45_WaitForTask_t waitForTask);

/**
 * @brief Reades data from ROM to external buffer
 * @param AT45_Handle: pointer to the device handle structure
 * @param buf: pointer to external buffer, that will contain the received data
 * @param dataLength: number of bytes to read
 * @param address: page address to read (multiple of 512 bytes)
 * @param trailingCRC: compare or not compare CRC at the end of frame
 * @return Device status
 */
AT45_Status_t AT45_Read(AT45_HandleTypeDef *AT45_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                        bool trailingCRC);

/**
 * @brief Begins erase operation of page, block, sector or whole memory array
 * @param AT45_Handle: pointer to the device handle structure
 * @param eraseInstruction: pages groups to be erased
 * @param address: start address of page, block or sector to be erased
 * @param waitForTask: the way to ensure that operation is completed
 * @return Device status
 * @note Address has to be 0 in case of chip erase
 */
AT45_Status_t AT45_Erase(AT45_HandleTypeDef *AT45_Handle, AT45_EraseInstruction_t eraseInstruction, uint32_t address,
                         AT45_WaitForTask_t waitForTask);

/**
 * @brief Checks if the device is busy or not
 * @param AT45_Handle: pointer to the device handle structure
 * @return True - device is busy
 */
bool AT45_Busy(AT45_HandleTypeDef *AT45_Handle);

/**
 * @brief Check whether the Chip is set to use binary (power of 2) page size or actual page size (slightly bigger)
 * @param AT45_Handle: pointer to the device handle structure
 * @return the currenly set page size type AT45_PAGE_SIZE_BINARY or AT45_PAGE_SIZE_ACTUAL
 */
enum AT45_PageSizeOptions_e AT45_PageSizeCheck(AT45_HandleTypeDef *AT45_Handle);
/**
 * @brief set the chip to use AT45_PAGE_SIZE_BINARY or AT45_PAGE_SIZE_ACTUAL
 * @param AT45_Handle: pointer to the device handle structure
 * @param targetPageSize: the page size option to use
 * @return SUCCESS or ERROR
 */
ErrorStatus AT45_PageSizeConfig(AT45_HandleTypeDef *AT45_Handle, enum AT45_PageSizeOptions_e targetPageSize);
/**
 * @brief returns the block size in bytes. 
 * the returned value can change for the same chip depending
 * on whether binary size is configured or not. always pagesize*8
 * @param AT45_Handle: pointer to the device handle structure
 */
int AT45_block_size(AT45_HandleTypeDef *AT45_Handle);
/**
 * @brief returns the sector size in bytes. 
 * the returned value can change for the same chip depending
 * on whether binary size is configured or not.
 * @param AT45_Handle: pointer to the device handle structure
 */
int AT45_sector_size(AT45_HandleTypeDef *AT45_Handle);

#endif