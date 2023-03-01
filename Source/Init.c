#include "Init.h"

void IO_Init(void)
{
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);

	//----- SPI1 -----//
	/* PA5 - "SCK" */
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE5, GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1);	// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF5, GPIO_CRL_CNF5_1);	// Alternate function output Push-pull
	/* PA7 - "MOSI" */
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE7, GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1);	// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF7, GPIO_CRL_CNF7_1);	// Alternate function output Push-pull
	/* PA6 - "MISO" */
	CLEAR_BIT(GPIOA->CRL, GPIO_CRL_MODE6);	// Input mode (reset state)
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF6, GPIO_CRL_CNF6_0);	// Floating input (reset state)
	/* PA4 - "NSS" */
	CS0_High;
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE4, GPIO_CRL_MODE4_0 | GPIO_CRL_MODE4_1);	// Output mode, max speed 50 MHz
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF4, GPIO_CRL_CNF4_0);	// General purpose output Open-drain

	//	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE4, GPIO_CRL_MODE4_0 | GPIO_CRL_MODE4_1);	// Output mode, max speed 50 MHz
	//	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF4, GPIO_CRL_CNF4_1);	// Alternate function output Push-pull
}