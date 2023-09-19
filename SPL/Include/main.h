#ifndef MAIN_H
#define MAIN_H

#include "AT45.h"
#include "init.h"
#include <stdbool.h>

/* Definitions */
// #define USE_IO_TERMINAL
#define PAGE         1408
#define PAGE_ADDRESS (PAGE * AT45_PAGE_SIZE)

/* Macro */
#ifndef USE_IO_TERMINAL
#define printf(param, ...) ((void) 0)
#endif

void Error_Handler(void);

#endif