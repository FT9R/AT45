#include "main.h"

/* Private variables */
static AT45_HandleTypeDef AT45_Handle;
static const uint8_t bufferWrite[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 'A', 'B', 'C'};
static uint8_t bufferRead[sizeof(bufferWrite)] = {0};

void main(void)
{
    Sys_Init();
    IO_Init();
    SPIx_Init(&hspi3, SPI_Mode_Master, SPI_BaudRatePrescaler_256);
    AT45_Init(&AT45_Handle, &hspi3, CS0_GPIO_Port, CS0_Pin);
    if (AT45_Handle.status == AT45_STATUS_READY)
    {
        // AT45_Erase(&AT45_Handle, AT45_CHIP_ERASE, NULL, AT45_WAIT_BUSY);
        printf("\r\n First approach to read \r\n");
        AT45_Read(&AT45_Handle, bufferRead, sizeof(bufferRead), PAGE_ADDRESS, true);
        if (strncmp((const char *) bufferRead, (const char *) bufferWrite, sizeof(bufferRead)) == 0)
        {
            printf("Data already exist at page %i boundaries \r\n", PAGE);
        }
        else
        {
            printf("Data doesn't exist at page %i boundaries \r\n", PAGE);
            printf("Page programming...");
            AT45_Write(&AT45_Handle, bufferWrite, sizeof(bufferWrite), PAGE_ADDRESS, true, true, AT45_WAIT_BUSY);
            printf("\r\n Second approach to read \r\n");
            AT45_Read(&AT45_Handle, bufferRead, sizeof(bufferRead), PAGE_ADDRESS, true);
            if (strncmp((const char *) bufferRead, (const char *) bufferWrite, sizeof(bufferRead)) == 0)
            {
                printf("Writing process success \r\n");
            }
            else
            {
                printf("Writing process failure \r\n");
                Error_Handler();
            }
        }
    }
    else
    {
        printf("Couldn't get any response from device \r\n");
        Error_Handler();
    }

    while (true) {}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {}
    /* USER CODE END Error_Handler_Debug */
}