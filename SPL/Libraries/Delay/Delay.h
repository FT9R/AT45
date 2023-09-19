#ifndef DELAY_H
#define DELAY_H

#include "stm32f4xx.h"

#define GLOBAL_TICK_FREQ 1 // 1 kHz

extern uint32_t uwTick;
extern void Error_Handler(void);

void Delay(uint32_t ms);

#endif