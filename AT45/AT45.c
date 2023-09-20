#include "AT45.h"

/* Private function prototypes */
static ErrorStatus AT45_ReadID(AT45_HandleTypeDef *AT45_Handle);
static void AT45_ReadStatus(AT45_HandleTypeDef *AT45_Handle);
static ErrorStatus AT45_WaitWithTimeout(AT45_HandleTypeDef *AT45_Handle, uint32_t timeout);
static ErrorStatus AT45_ConfigurePageSize(AT45_HandleTypeDef *AT45_Handle, uint16_t pageSize);
static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize);

ErrorStatus AT45_Init(AT45_HandleTypeDef *AT45_Handle, SPI_HandleTypeDef *hspix, GPIO_TypeDef *CS_Port, uint16_t CS_Pin)
{
    AT45_Delay(100);

    /* SPI device specific info retrieve */
    AT45_Handle->hspix = hspix;
    AT45_Handle->CS_Port = CS_Port;
    AT45_Handle->CS_Pin = CS_Pin;
    AT45_Handle->status = ERROR;

    /* Check for SPI1-3 match */
    if ((AT45_Handle->hspix->Instance != SPI1) && (AT45_Handle->hspix->Instance != SPI2)
        && (AT45_Handle->hspix->Instance != SPI3))
        return AT45_Handle->status;

    /* Get the ManufacturerID and DeviceID */
    AT45_ReadID(AT45_Handle);
    if (AT45_Handle->ID[0] != AT45_MANUFACTURER_ID)
        return AT45_Handle->status;

    /* Force page size to 512 */
    /* Check actual page size to prevent override */
    AT45_ReadStatus(AT45_Handle);
    if (!READ_BIT(AT45_Handle->statusRegister[0], 1 << 0))
    {
        if (AT45_ConfigurePageSize(AT45_Handle, 512) != SUCCESS)
            return AT45_Handle->status;
    }

    /* Determine number of pages */
    switch (AT45_Handle->ID[1])
    {
    case AT45DB161:
        AT45_Handle->numberOfPages = 16 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / AT45_PAGE_SIZE / 8);
        break;

    default:
        return AT45_Handle->status; // Unsupported device
    }

    AT45_Delay(10);

    return AT45_Handle->status = SUCCESS;
}

ErrorStatus AT45_Write(AT45_HandleTypeDef *AT45_Handle, const uint8_t *buf, uint16_t dataLength, uint32_t address,
                       bool trailingCRC, bool pageErase, waitForTask_t waitForTask)
{
    AT45_Handle->status = ERROR;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return AT45_Handle->status;
    if (trailingCRC)
        dataLength += sizeof(uint16_t);
    if (dataLength > AT45_PAGE_SIZE)
        return AT45_Handle->status;
    if ((address % AT45_PAGE_SIZE) != 0)
        return AT45_Handle->status; // Only first byte of the page can be pointed as the start byte
    if (address > (AT45_PAGE_SIZE * (AT45_Handle->numberOfPages - 1)))
        return AT45_Handle->status; // The boundaries of write operation beyond memory

    /* Middle buffer operations */
    uint8_t *midBuf = malloc(sizeof(*midBuf) * dataLength);
    if (midBuf == NULL)
        return AT45_Handle->status; // Insufficient heap memory available
    if (trailingCRC)
    {
        /* Data + checksum */
        memcpy(midBuf, buf, dataLength - sizeof(uint16_t));
        CRC16 = ModBus_CRC(buf, dataLength - sizeof(uint16_t));
        midBuf[dataLength - 2] = ((CRC16 >> 8) & 0xFF);
        midBuf[dataLength - 1] = (CRC16 & 0xFF);
    }
    else
        /* Pure data */
        memcpy(midBuf, buf, dataLength);

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status;

    /* Buffer write */
    /* Command */
    CS_LOW(AT45_Handle);
    AT45_Handle->CMD[0] = AT45_CMD_BUFFER_1_WRITE;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* BFA8-BFA0 - Address of the first byte in the SRAM buffer to be written */
    /* Fixed zero position */
    ADDRESS_BYTES_SWAP(AT45_Handle, 0);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                      AT45_TX_TIMEOUT);

    /* AT45 SRAM buffer filling by user data */
    AT45_SPI_Transmit(AT45_Handle->hspix, midBuf, dataLength, AT45_TX_TIMEOUT);
    free(midBuf);
    CS_HIGH(AT45_Handle);

    /* Page program */
    /* Command */
    CS_LOW(AT45_Handle);
    if (pageErase)
        AT45_Handle->CMD[0] = AT45_CMD_BUFFER_1_TO_MAIN_MEMORY_PAGE_PROGRAM_ERASE;
    else
        AT45_Handle->CMD[0] = AT45_CMD_BUFFER_1_TO_MAIN_MEMORY_PAGE_PROGRAM;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* A20-A9 - 12 page address bits that specify the page in the main memory to be written */
    ADDRESS_BYTES_SWAP(AT45_Handle, address);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                      AT45_TX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    /* Wait options */
    if (waitForTask == AT45_WAIT_DELAY)
    {
        if (pageErase)
            AT45_Delay(AT45_PAGE_ERASE_PROGRAMMING_TIME);
        else
            AT45_Delay(AT45_PAGE_PROGRAMMING_TIME);
    }
    else if (waitForTask == AT45_WAIT_BUSY)
    {
        if (pageErase)
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_PAGE_ERASE_PROGRAMMING_TIME) != SUCCESS)
                return AT45_Handle->status;
        }
        else
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_PAGE_PROGRAMMING_TIME) != SUCCESS)
                return AT45_Handle->status;
        }
    }

    return AT45_Handle->status = SUCCESS;
}

ErrorStatus AT45_Read(AT45_HandleTypeDef *AT45_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                      bool trailingCRC)
{
    AT45_Handle->status = ERROR;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return AT45_Handle->status;
    if (trailingCRC)
        dataLength += sizeof(uint16_t);
    if (dataLength > AT45_PAGE_SIZE)
        return AT45_Handle->status;
    if ((address % AT45_PAGE_SIZE) != 0)
        return AT45_Handle->status; // Only first byte of the page can be pointed as the start byte
    if (address > (AT45_PAGE_SIZE * (AT45_Handle->numberOfPages - 1)))
        return AT45_Handle->status; // The boundaries of read operation beyond memory

    /* Middle buffer operations */
    uint8_t *midBuf = malloc(sizeof(*midBuf) * dataLength);
    if (midBuf == NULL)
        return AT45_Handle->status; // Insufficient heap memory available

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status;

    /* Command */
    CS_LOW(AT45_Handle);
    AT45_Handle->CMD[0] = AT45_CMD_MAIN_MEMORY_PAGE_READ;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* A20-A9 - 12 page address bits that specify the page in the main memory to be read */
    ADDRESS_BYTES_SWAP(AT45_Handle, address);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                      AT45_TX_TIMEOUT);

    /* 4 dummy bytes */
    AT45_Handle->CMD[0] = NULL;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* Data read */
    AT45_SPI_Receive(AT45_Handle->hspix, midBuf, dataLength, AT45_RX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    /* Checksum compare */
    if (trailingCRC)
    {
        CRC16 = ModBus_CRC(midBuf, dataLength - sizeof(uint16_t));
        if ((midBuf[dataLength - 2] != ((CRC16 >> 8) & 0xFF)) || (midBuf[dataLength - 1] != (CRC16 & 0xFF)))
            return AT45_Handle->status; // CRC error
    }

    /* Copy middle buffer content to the destination buffer */
    if (trailingCRC)
        memcpy(buf, midBuf, dataLength - sizeof(uint16_t));
    else
        memcpy(buf, midBuf, dataLength);
    free(midBuf);

    return AT45_Handle->status = SUCCESS;
}

ErrorStatus AT45_Erase(AT45_HandleTypeDef *AT45_Handle, eraseInstruction_t eraseInstruction, uint32_t address,
                       waitForTask_t waitForTask)
{
    AT45_Handle->status = ERROR;

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status;

    switch (eraseInstruction)
    {
    case AT45_PAGE_ERASE:
        if ((address % AT45_PAGE_SIZE) != 0)
            return AT45_Handle->status; // Incorrect start address for page
        if (address > ((AT45_PAGE_SIZE * AT45_Handle->numberOfPages) - AT45_PAGE_SIZE))
            return AT45_Handle->status; // The boundaries of erase operation beyond memory

        /* Command */
        CS_LOW(AT45_Handle);
        AT45_Handle->CMD[0] = AT45_CMD_PAGE_ERASE;
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

        /* A20-A9 - 12 page address bits that specify the page in the main memory to be erased */
        ADDRESS_BYTES_SWAP(AT45_Handle, address);
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                          AT45_TX_TIMEOUT);
        CS_HIGH(AT45_Handle);

        /* Wait options */
        if (waitForTask == AT45_WAIT_DELAY)
            AT45_Delay(AT45_PAGE_ERASE_TIME);
        else if (waitForTask == AT45_WAIT_BUSY)
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_PAGE_ERASE_TIME) != SUCCESS)
                return AT45_Handle->status;
        }
        break;

    case AT45_BLOCK_ERASE: // 1 block - 8 pages
        if ((address % (AT45_PAGE_SIZE * 8)) != 0)
            return AT45_Handle->status; // Incorrect start address for block
        if (address > ((AT45_PAGE_SIZE * AT45_Handle->numberOfPages) - AT45_PAGE_SIZE * 8))
            return AT45_Handle->status; // The boundaries of erase operation beyond memory

        /* Command */
        CS_LOW(AT45_Handle);
        AT45_Handle->CMD[0] = AT45_CMD_BLOCK_ERASE;
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

        /* A20-A12 - 9 block address bits that specify the block in the main memory to be erased */
        ADDRESS_BYTES_SWAP(AT45_Handle, address);
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                          AT45_TX_TIMEOUT);
        CS_HIGH(AT45_Handle);

        /* Wait options */
        if (waitForTask == AT45_WAIT_DELAY)
            AT45_Delay(AT45_BLOCK_ERASE_TIME);
        else if (waitForTask == AT45_WAIT_BUSY)
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_BLOCK_ERASE_TIME) != SUCCESS)
                return AT45_Handle->status;
        }
        break;

    case AT45_SECTOR_ERASE: // 1 sector - 256 pages
        if ((address % (AT45_PAGE_SIZE * 256)) != 0)
            return AT45_Handle->status; // Incorrect start address for sector
        if (address > ((AT45_PAGE_SIZE * AT45_Handle->numberOfPages) - AT45_PAGE_SIZE * 256))
            return AT45_Handle->status; // The boundaries of erase operation beyond memory

        /* Command */
        CS_LOW(AT45_Handle);
        AT45_Handle->CMD[0] = AT45_CMD_SECTOR_ERASE;
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

        /* A20-A12 - 9 sector address bits that specify the sector in the main memory to be erased */
        ADDRESS_BYTES_SWAP(AT45_Handle, address);
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                          AT45_TX_TIMEOUT);
        CS_HIGH(AT45_Handle);

        /* Wait options */
        if (waitForTask == AT45_WAIT_DELAY)
            AT45_Delay(AT45_SECTOR_ERASE_TIME);
        else if (waitForTask == AT45_WAIT_BUSY)
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_SECTOR_ERASE_TIME) != SUCCESS)
                return AT45_Handle->status;
        }
        break;

    case AT45_CHIP_ERASE:
        if (address != NULL)
            return AT45_Handle->status;

        /* Command */
        CS_LOW(AT45_Handle);
        AT45_Handle->CMD[0] = AT45_CMD_CHIP_ERASE_0;
        AT45_Handle->CMD[1] = AT45_CMD_CHIP_ERASE_1;
        AT45_Handle->CMD[2] = AT45_CMD_CHIP_ERASE_2;
        AT45_Handle->CMD[3] = AT45_CMD_CHIP_ERASE_3;
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD), AT45_TX_TIMEOUT);
        CS_HIGH(AT45_Handle);

        /* Wait options */
        if (waitForTask == AT45_WAIT_DELAY)
            AT45_Delay(AT45_CHIP_ERASE_TIME);
        else if (waitForTask == AT45_WAIT_BUSY)
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_CHIP_ERASE_TIME) != SUCCESS)
                return AT45_Handle->status;
        }
        break;

    default:
        return AT45_Handle->status;
    }
    return AT45_Handle->status = SUCCESS;
}

bool AT45_Busy(AT45_HandleTypeDef *AT45_Handle)
{
    AT45_ReadStatus(AT45_Handle);

    return !READ_BIT(AT45_Handle->statusRegister[0], 1 << 7);
}

/**
 * @section Private functions
 */
static ErrorStatus AT45_ReadID(AT45_HandleTypeDef *AT45_Handle)
{
    AT45_Handle->status = ERROR;

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status;

    CS_LOW(AT45_Handle);
    AT45_Handle->CMD[0] = AT45_CMD_MANUFACTURER_DEVICE_ID_READ;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Receive(AT45_Handle->hspix, AT45_Handle->ID, sizeof(AT45_Handle->ID), AT45_RX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    return AT45_Handle->status = SUCCESS;
}

static void AT45_ReadStatus(AT45_HandleTypeDef *AT45_Handle)
{
    CS_LOW(AT45_Handle);
    AT45_Handle->CMD[0] = AT45_CMD_STATUS_REGISTER_READ;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Receive(AT45_Handle->hspix, AT45_Handle->statusRegister, sizeof(AT45_Handle->statusRegister),
                     AT45_RX_TIMEOUT);

    CS_HIGH(AT45_Handle);
}

static ErrorStatus AT45_WaitWithTimeout(AT45_HandleTypeDef *AT45_Handle, uint32_t timeout)
{
    uint32_t tickStart = uwTick;

    while ((uwTick - tickStart) < timeout)
    {
        if (!AT45_Busy(AT45_Handle))
            return SUCCESS;
    }
    return ERROR;
}

static ErrorStatus AT45_ConfigurePageSize(AT45_HandleTypeDef *AT45_Handle, uint16_t pageSize)
{
    if (pageSize == 512)
    {
        AT45_Handle->CMD[0] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_0;
        AT45_Handle->CMD[1] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_1;
        AT45_Handle->CMD[2] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_2;
        AT45_Handle->CMD[3] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_3;
    }
    else if (pageSize == 528)
    {
        AT45_Handle->CMD[0] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_0;
        AT45_Handle->CMD[1] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_1;
        AT45_Handle->CMD[2] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_2;
        AT45_Handle->CMD[3] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_3;
    }
    else
        return ERROR; // Incorrect page size

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return ERROR;

    /* Page size configuration */
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD), AT45_TX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    /* Wait for end of programming of the nonvolatile register */
    if (AT45_WaitWithTimeout(AT45_Handle, AT45_PAGE_PROGRAMMING_TIME) != SUCCESS)
        return ERROR;
    /* Check again the page size */
    AT45_ReadStatus(AT45_Handle);
    if (!READ_BIT(AT45_Handle->statusRegister[0], 1 << 0))
        return ERROR;

    return SUCCESS;
}

static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize)
{
    uint16_t CRC16 = 0xffff;
    uint16_t i, j;
    for (i = 0; i < bufSize; i++)
    {
        CRC16 ^= pBuffer[i];
        for (j = 0; j < 8; j++)
        {
            if (CRC16 & 1)
                CRC16 = (CRC16 >> 1) ^ 0xA001;
            else
                CRC16 = (CRC16 >> 1);
        }
    }
    return CRC16;
}