#ifndef INIT_H
#define INIT_H

#include "SPI.h"
#include "sys_init.h"

#define CS0_Pin       GPIO_Pin_0
#define CS0_GPIO_Port GPIOD

extern SPI_HandleTypeDef hspi3;

void IO_Init(void);

#endif