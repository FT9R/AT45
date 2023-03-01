#ifndef DELAY_H
#define DELAY_H

//#define	NDEBUG

#include "stm32f10x.h"
#include <assert.h>

void _delay_ms(const uint32_t ms);
void _delay_us(const uint32_t us);

#endif