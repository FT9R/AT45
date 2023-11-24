#include "AT45.h"

/* Private function prototypes */
static void AT45_ReadID(AT45_HandleTypeDef *AT45_Handle);
static void AT45_ReadStatus(AT45_HandleTypeDef *AT45_Handle);
static ErrorStatus AT45_WaitWithTimeout(AT45_HandleTypeDef *AT45_Handle, uint32_t timeout);
static uint16_t AT45_PageSizeCheck(AT45_HandleTypeDef *AT45_Handle);
static ErrorStatus AT45_PageSizeConfig(AT45_HandleTypeDef *AT45_Handle, uint16_t targetPageSize);
static uint16_t ModBus_CRC(const uint8_t *pBuffer, uint16_t bufSize);

AT45_Status_t AT45_Init(AT45_HandleTypeDef *AT45_Handle, SPI_HandleTypeDef *hspix, GPIO_TypeDef *CS_Port,
                        uint16_t CS_Pin)
{
    /* Argument guards */
    if (AT45_Handle == NULL)
        return AT45_STATUS_ERROR_ARGUMENT;
    if (hspix == NULL)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if (CS_Port == NULL)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if (CS_Pin == 0x0000)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

    AT45_Delay(100);

    /* SPI device specific info retrieve */
    AT45_Handle->hspix = hspix;
    AT45_Handle->CS_Port = CS_Port;
    AT45_Handle->CS_Pin = CS_Pin;
    AT45_Handle->status = AT45_STATUS_RESET;

    /* Check for SPI1-3 match */
    if ((AT45_Handle->hspix->Instance != SPI1) && (AT45_Handle->hspix->Instance != SPI2) &&
        (AT45_Handle->hspix->Instance != SPI3))
        return AT45_Handle->status = AT45_STATUS_ERROR_INITIALIZATION;

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;

    /* Get the ManufacturerID and DeviceID */
    AT45_ReadID(AT45_Handle);
    if (AT45_Handle->ID[0] != AT45_MANUFACTURER_ID)
        return AT45_Handle->status = AT45_STATUS_ERROR_INITIALIZATION;

    /* Determine number of pages */
    switch (AT45_Handle->ID[1])
    {
    case AT45DB161:
        AT45_Handle->numberOfPages = 16 * (KB_TO_BYTE(1) * KB_TO_BYTE(1) / AT45_PAGE_SIZE / 8);
        break;

    default:
        return AT45_Handle->status = AT45_STATUS_ERROR_INITIALIZATION;
    }

    /* Force page size to binary */
    if (AT45_PageSizeCheck(AT45_Handle) != AT45_PAGE_SIZE)
    {
        if (AT45_PageSizeConfig(AT45_Handle, AT45_PAGE_SIZE) != SUCCESS)
            return AT45_Handle->status = AT45_STATUS_ERROR_INITIALIZATION;
    }

    AT45_Delay(10);

    return AT45_Handle->status = AT45_STATUS_READY;
}

AT45_Status_t AT45_Write(AT45_HandleTypeDef *AT45_Handle, const uint8_t *buf, uint16_t dataLength, uint32_t address,
                         bool trailingCRC, bool pageErase, AT45_WaitForTask_t waitForTask)
{
    AT45_Handle->status = AT45_STATUS_BUSY_WRITE;
    uint16_t frameLength = dataLength;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if (trailingCRC)
        frameLength += sizeof(CRC16);
    if (frameLength > AT45_PAGE_SIZE)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if ((address % AT45_PAGE_SIZE) != 0)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if (address > (AT45_PAGE_SIZE * (AT45_Handle->numberOfPages - 1)))
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

    /* Checksum calculate */
    if (trailingCRC)
        CRC16 = ModBus_CRC(buf, dataLength);

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;

    /* Buffer write */
    /* Command */
    AT45_Handle->CMD[0] = AT45_CMD_BUFFER_1_WRITE;
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* BFA8-BFA0 - Address of the first byte in the SRAM buffer to be written */
    /* Fixed zero position */
    ADDRESS_BYTES_SWAP(AT45_Handle, 0);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                      AT45_TX_TIMEOUT);

    /* AT45 SRAM buffer filling by user data */
    /* Data */
    AT45_SPI_Transmit(AT45_Handle->hspix, (uint8_t *) buf, dataLength, AT45_TX_TIMEOUT);

    /* Checksum */
    if (trailingCRC)
        AT45_SPI_Transmit(AT45_Handle->hspix, (uint8_t *) &CRC16, sizeof(CRC16), AT45_TX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    /* Page program */
    /* Command */
    if (pageErase)
        AT45_Handle->CMD[0] = AT45_CMD_BUFFER_1_TO_MAIN_MEMORY_PAGE_PROGRAM_ERASE;
    else
        AT45_Handle->CMD[0] = AT45_CMD_BUFFER_1_TO_MAIN_MEMORY_PAGE_PROGRAM;
    CS_LOW(AT45_Handle);
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
                return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;
        }
        else
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_PAGE_PROGRAMMING_TIME) != SUCCESS)
                return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;
        }
    }

    return AT45_Handle->status = AT45_STATUS_READY;
}

AT45_Status_t AT45_Read(AT45_HandleTypeDef *AT45_Handle, uint8_t *buf, uint16_t dataLength, uint32_t address,
                        bool trailingCRC)
{
    AT45_Handle->status = AT45_STATUS_BUSY_READ;
    uint16_t frameLength = dataLength;
    uint16_t CRC16 = 0x0000;

    /* Argument guards */
    if ((dataLength == 0) || (buf == NULL))
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if (trailingCRC)
        frameLength += sizeof(CRC16);
    if (frameLength > AT45_PAGE_SIZE)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if ((address % AT45_PAGE_SIZE) != 0)
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
    if (address > (AT45_PAGE_SIZE * (AT45_Handle->numberOfPages - 1)))
        return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;

    /* Frame buffer memory allocation */
    uint8_t *frameBuf = malloc(sizeof(*frameBuf) * frameLength);
    if (frameBuf == NULL)
        return AT45_Handle->status = AT45_STATUS_ERROR_MEM_MANAGE;

    /* Command */
    AT45_Handle->CMD[0] = AT45_CMD_MAIN_MEMORY_PAGE_READ;
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* A20-A9 - 12 page address bits that specify the page in the main memory to be read */
    ADDRESS_BYTES_SWAP(AT45_Handle, address);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->addressBytes, sizeof(AT45_Handle->addressBytes),
                      AT45_TX_TIMEOUT);

    /* 4 dummy bytes */
    AT45_Handle->CMD[0] = 0;
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    /* Data receive */
    AT45_SPI_Receive(AT45_Handle->hspix, frameBuf, frameLength, AT45_RX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    /* Checksum compare */
    if (trailingCRC)
    {
        CRC16 = ModBus_CRC(frameBuf, dataLength);
        if (memcmp(&frameBuf[dataLength], &CRC16, sizeof(CRC16)) != 0)
        {
            free(frameBuf);
            return AT45_Handle->status = AT45_STATUS_ERROR_CHECKSUM;
        }
    }

    /* Copy received data without checksum to the destination buffer */
    memcpy(buf, frameBuf, dataLength);
    free(frameBuf);

    return AT45_Handle->status = AT45_STATUS_READY;
}

AT45_Status_t AT45_Erase(AT45_HandleTypeDef *AT45_Handle, AT45_EraseInstruction_t eraseInstruction, uint32_t address,
                         AT45_WaitForTask_t waitForTask)
{
    AT45_Handle->status = AT45_STATUS_BUSY_ERASE;

    if (AT45_WaitWithTimeout(AT45_Handle, AT45_RESPONSE_TIMEOUT) != SUCCESS)
        return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;

    switch (eraseInstruction)
    {
    case AT45_PAGE_ERASE:
        if ((address % AT45_PAGE_SIZE) != 0)
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
        if (address > ((AT45_PAGE_SIZE * AT45_Handle->numberOfPages) - AT45_PAGE_SIZE))
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

        /* Command */
        AT45_Handle->CMD[0] = AT45_CMD_PAGE_ERASE;
        CS_LOW(AT45_Handle);
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
                return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;
        }
        break;

    case AT45_BLOCK_ERASE:
        if ((address % AT45_BLOCK_SIZE) != 0)
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
        if (address > ((AT45_PAGE_SIZE * AT45_Handle->numberOfPages) - AT45_BLOCK_SIZE))
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

        /* Command */
        AT45_Handle->CMD[0] = AT45_CMD_BLOCK_ERASE;
        CS_LOW(AT45_Handle);
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
                return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;
        }
        break;

    case AT45_SECTOR_ERASE:
        if ((address % AT45_SECTOR_SIZE) != 0)
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;
        if (address > ((AT45_PAGE_SIZE * AT45_Handle->numberOfPages) - AT45_SECTOR_SIZE))
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

        /* Command */
        AT45_Handle->CMD[0] = AT45_CMD_SECTOR_ERASE;
        CS_LOW(AT45_Handle);
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
                return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;
        }
        break;

    case AT45_CHIP_ERASE:
        if (address != 0)
            return AT45_Handle->status = AT45_STATUS_ERROR_ARGUMENT;

        /* Command */
        AT45_Handle->CMD[0] = AT45_CMD_CHIP_ERASE_0;
        AT45_Handle->CMD[1] = AT45_CMD_CHIP_ERASE_1;
        AT45_Handle->CMD[2] = AT45_CMD_CHIP_ERASE_2;
        AT45_Handle->CMD[3] = AT45_CMD_CHIP_ERASE_3;
        CS_LOW(AT45_Handle);
        AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD), AT45_TX_TIMEOUT);
        CS_HIGH(AT45_Handle);

        /* Wait options */
        if (waitForTask == AT45_WAIT_DELAY)
            AT45_Delay(AT45_CHIP_ERASE_TIME);
        else if (waitForTask == AT45_WAIT_BUSY)
        {
            if (AT45_WaitWithTimeout(AT45_Handle, AT45_CHIP_ERASE_TIME) != SUCCESS)
                return AT45_Handle->status = AT45_STATUS_ERROR_TIMEOUT;
        }
        break;

    default:
        return AT45_Handle->status = AT45_STATUS_ERROR_INSTRUCTION;
    }

    return AT45_Handle->status = AT45_STATUS_READY;
}

bool AT45_Busy(AT45_HandleTypeDef *AT45_Handle)
{
    AT45_ReadStatus(AT45_Handle);

    return !READ_BIT(AT45_Handle->statusRegister[0], 1u << 7);
}

/**
 * @section Private functions
 */
static void AT45_ReadID(AT45_HandleTypeDef *AT45_Handle)
{
    AT45_Handle->CMD[0] = AT45_CMD_MANUFACTURER_DEVICE_ID_READ;
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Receive(AT45_Handle->hspix, AT45_Handle->ID, sizeof(AT45_Handle->ID), AT45_RX_TIMEOUT);
    CS_HIGH(AT45_Handle);
}

static void AT45_ReadStatus(AT45_HandleTypeDef *AT45_Handle)
{
    AT45_Handle->CMD[0] = AT45_CMD_STATUS_REGISTER_READ;
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);
    AT45_SPI_Receive(AT45_Handle->hspix, AT45_Handle->statusRegister, sizeof(AT45_Handle->statusRegister),
                     AT45_RX_TIMEOUT);
    CS_HIGH(AT45_Handle);
}

static ErrorStatus AT45_WaitWithTimeout(AT45_HandleTypeDef *AT45_Handle, uint32_t timeout)
{
    uint32_t tickStart = uwTick;

    /* Command */
    AT45_Handle->CMD[0] = AT45_CMD_STATUS_REGISTER_READ;
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD[0]), AT45_TX_TIMEOUT);

    while ((uwTick - tickStart) < timeout)
    {
        /* Get busy bit state */
        AT45_SPI_Receive(AT45_Handle->hspix, AT45_Handle->statusRegister, sizeof(AT45_Handle->statusRegister),
                         AT45_RX_TIMEOUT);
        if (READ_BIT(AT45_Handle->statusRegister[0], 1u << 7))
        {
            CS_HIGH(AT45_Handle);
            return SUCCESS;
        }
    }
    CS_HIGH(AT45_Handle);

    return ERROR;
}

static uint16_t AT45_PageSizeCheck(AT45_HandleTypeDef *AT45_Handle)
{
    AT45_ReadStatus(AT45_Handle);

    return READ_BIT(AT45_Handle->statusRegister[0], 1u << 0) ? 512 : 528;
}

static ErrorStatus AT45_PageSizeConfig(AT45_HandleTypeDef *AT45_Handle, uint16_t targetPageSize)
{
    if (targetPageSize == 512)
    {
        AT45_Handle->CMD[0] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_0;
        AT45_Handle->CMD[1] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_1;
        AT45_Handle->CMD[2] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_2;
        AT45_Handle->CMD[3] = AT45_CMD_CONFIGURE_BINARY_PAGE_SIZE_3;
    }
    else if (targetPageSize == 528)
    {
        AT45_Handle->CMD[0] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_0;
        AT45_Handle->CMD[1] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_1;
        AT45_Handle->CMD[2] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_2;
        AT45_Handle->CMD[3] = AT45_CMD_CONFIGURE_STANDART_PAGE_SIZE_3;
    }
    else
        return ERROR;

    /* Page size configuration */
    CS_LOW(AT45_Handle);
    AT45_SPI_Transmit(AT45_Handle->hspix, AT45_Handle->CMD, sizeof(AT45_Handle->CMD), AT45_TX_TIMEOUT);
    CS_HIGH(AT45_Handle);

    /* Wait for end of programming of the nonvolatile register */
    if (AT45_WaitWithTimeout(AT45_Handle, AT45_PAGE_ERASE_PROGRAMMING_TIME) != SUCCESS)
        return ERROR;

    /* Check the new page size */
    if (AT45_PageSizeCheck(AT45_Handle) != targetPageSize)
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