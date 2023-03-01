#ifndef MAIN_H
#define MAIN_H

#define USE_IO_TERMINAL
#define PAGE_TO_WRITE 1408

#include "stm32f10x.h"
#include "Sys_Init.h"
#include <string.h>
#include <stdio.h>
#include "Delay.h"
#include "Init.h"
#include "SPI.h"
#include "AT45.h"

#ifndef USE_IO_TERMINAL
printf(param) ((void)0)
#endif

#endif