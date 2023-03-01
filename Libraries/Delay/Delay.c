#include "Delay.h"

void _delay_ms(const uint32_t ms)
{
	if(ms > (16777215 / (SystemCoreClock / 1000))) {assert(ms < (16777215 / (SystemCoreClock / 1000))); return;}
	
	MODIFY_REG(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk, SysTick_CTRL_CLKSOURCE_Msk);	//CLKSOURCE: 1 Ц HCLK
	SysTick->VAL = 0;	// A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the STK_CTRL register to 0	
	SysTick->LOAD = ((SystemCoreClock / 1000) * ms) - 1;
	MODIFY_REG(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, SysTick_CTRL_ENABLE_Msk);	// 1: Counter enabled
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) {asm("nop");}
	SysTick->CTRL = (uint32_t)0x00000000;	// 0: Counter disabled
}

void _delay_us(const uint32_t us)
{
	if(us > (16777215 / (SystemCoreClock / 1000000))) {assert(us < (16777215 / (SystemCoreClock / 1000000))); return;}
	
	MODIFY_REG(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk, SysTick_CTRL_CLKSOURCE_Msk);	//CLKSOURCE: 1 Ц HCLK
	SysTick->VAL = 0;	// A write of any value clears the field to 0, and also clears the COUNTFLAG bit in the STK_CTRL register to 0	
	SysTick->LOAD = ((SystemCoreClock / 1000000) * us) - 1;
	MODIFY_REG(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk, SysTick_CTRL_ENABLE_Msk);	// 1: Counter enabled
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) {asm("nop");}
	SysTick->CTRL = (uint32_t)0x00000000;	// 0: Counter disabled
}


/*	//____________ Legacy based on interrupts ____________//
uint32_t Delay_count_ms, Delay_count_us;


void SysTick_Init (void)
{
SysTick->CTRL |= (1 << 2);	//CLKSOURCE: 1 Ц HCLK
SysTick->CTRL |= (1 << 1);	//TCKINT: 1 - таймер досчитывает до нул€ и возникает запрос на прерывание
}

void _delay_ms(uint32_t ms)
{
SysTick_Init();
SysTick->VAL = 10;					//«апись любого значени€ очищает регистр в ноль, и также очищает бит COUNTFLAG регистра CTRL		
SysTick->LOAD = (F_CPU / 1000) - 1;
SysTick->CTRL |= (1 << 0);	//ENABLE: 1 Ц работа таймера разрешена
Delay_count_ms = ms;
while(Delay_count_ms) {asm("nop");}
SysTick->CTRL &= ~(1 << 0);	//ENABLE: 0 Ц работа таймера запрещена
}

void _delay_us(uint32_t us)
{
SysTick_Init();	
SysTick->VAL = 10;					//«апись любого значени€ очищает регистр в ноль, и также очищает бит COUNTFLAG регистра CTRL	
SysTick->LOAD = (F_CPU / 1000000) - 1;
SysTick->CTRL |= (1 << 0);	//ENABLE: 1 Ц работа таймера разрешен
Delay_count_us = us;
while(Delay_count_us) {asm("nop");}
SysTick->CTRL &= ~(1 << 0);	//ENABLE: 0 Ц работа таймера запрещена
}

void SysTick_Handler(void)
{
if (Delay_count_ms > 0) Delay_count_ms--;
if (Delay_count_us > 0) Delay_count_us--;
}
*/
