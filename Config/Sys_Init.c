#include "Sys_Init.h"

RCC_ClocksTypeDef rcc_clocks;


// setting F_CPU to 72 MHz, assuming that external crystal freq = 8 MHz
void Sys_Init(void)
{
	SystemInit();

	SET_BIT(FLASH->ACR, FLASH_ACR_PRFTBE);	// Enable Prefetch Buffer
	MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, 1 << 1);	// 010:Two wait states, if 48 MHz < SYSCLK <= 72 MHz

	SET_BIT(RCC->CR, RCC_CR_HSEON);	// HSE oscillator ON
	while(!READ_BIT(RCC->CR, RCC_CR_HSERDY));	// wait until HSE oscillator is ready;
	CLEAR_BIT(RCC->CFGR2, RCC_CFGR2_PREDIV1SRC);	// HSE oscillator clock selected as PREDIV1 clock entry
	MODIFY_REG(RCC->CFGR2, RCC_CFGR2_PREDIV1, RCC_CFGR2_PREDIV1_DIV1);	// PREDIV1 input clock not divided
	SET_BIT(RCC->CFGR, RCC_CFGR_PLLSRC);	// Clock from PREDIV1 selected as PLL input clock
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PLLMULL, RCC_CFGR_PLLMULL9);	//  PLL input clock x 9
	SET_BIT(RCC->CR, RCC_CR_PLLON);	// PLL ON
	while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY));	// wait until PLL is ready

	MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);	// PLL selected as system clock

	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1);	// AHB prescaler
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV2);	// APB low-speed prescaler (APB1)
	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV1);	// APB high-speed prescaler (APB2)

	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);	// wait until system will use PLL as system clock

	SystemCoreClockUpdate();
	RCC_GetClocksFreq(&rcc_clocks);
}
