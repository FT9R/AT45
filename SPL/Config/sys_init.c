#include "sys_init.h"

RCC_ClocksTypeDef rcc_clocks;

void Sys_Init(void)
{
    RCC_DeInit();
    SystemInit();

    FLASH_PrefetchBufferCmd(ENABLE);
    FLASH_SetLatency(FLASH_ACR_LATENCY_5WS);

    RCC_HSEConfig(RCC_HSE_ON);
    RCC_WaitForHSEStartUp();
    RCC_ClockSecuritySystemCmd(ENABLE);

    RCC_PLLConfig(RCC_PLLSource_HSE, 4, 168, 2, 7);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) != SET) {}

    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);
    RCC_PCLK1Config(RCC_HCLK_Div4);
    RCC_PCLK2Config(RCC_HCLK_Div2);
    while (RCC_GetSYSCLKSource() != RCC_CFGR_SWS_PLL) {}

    SystemCoreClockUpdate();
    RCC_GetClocksFreq(&rcc_clocks);
}