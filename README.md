# Description
A simple library designed to perform basic write/read operations with serial flash memory devices of the AT45DB family. 
## Features
* Many devices on the same bus are supported with its dedicated handles:
```C
/* First device */
AT45_Init(&AT45_Handle, &hspi3, CS0_GPIO_Port, CS0_Pin);

/* Second device */
AT45_Init(&AT45_Handle1, &hspi3, CS1_GPIO_Port, CS1_Pin);

/* Third device */
AT45_Init(&AT45_Handle2, &hspi3, CS2_GPIO_Port, CS2_Pin);
```
* Data transfer is carried out by standard SPI instructions, using the CLK, /CS, DI, DO pins.  
* Based on the device ID this library can calculate the number of pages to eliminate some address issues for write/read and erase operations.
* Parameter `pageErase` of AT45_Write function helps to meet different scenarios, especially in time-critical procedures.
* There are several options for waiting for the end of page program/erase instruction with dedicated timeouts.
* The built-in ModBus CRC can be used to ensure data integrity.
* The binary page size is forced for convenience.
* Buffer 1 is used only.
## Note
To make the use of the library as safe and understandable as possible, any operations with data are performed only starting from the first byte of the page 
(e.g., for the first page the address should be 0, for the second page - 512, etc.).   

# Supported devices
* AT45DB161E

# Quick start
## Common routine
* Don't forget the following line:
```C
#include "AT45.h"
```
* Declare the device handle:
```C
AT45_HandleTypeDef AT45_Handle;
```
* Provide defines regarding to Chip Select pin:
```C
#define CS0_Pin       GPIO_PIN_0
#define CS0_GPIO_Port GPIOD
```
* Initialize SPIx periphery
* Initialize the FLASH device:
```C
AT45_Init(&AT45_Handle, &hspi3, CS0_GPIO_Port, CS0_Pin);
```
## Interfacing with HAL
This library should work out of box together with HAL 
## Interfacing with SPL
* In `AT45_Interface.h` provide your own `SPI.h` and `Delay.h` includes   
* In `AT45_Interface.c` change next func calls to yours:
```C
SPI_Transmit(hspix, pData, size, timeout);
///
SPI_Receive(hspix, pData, size, timeout);
///
Delay(ms);
```
Or just use existing SPL SPI driver, which are oversimplificated but still in manner of HAL driver
# Example
## Conditions
`Toolchain - IAR EWARM v9.40.1`  
`Target MCU - STM32F407VGT6`
## References
For application use refer to [`HAL/../main.c`](./HAL/Core/Src/main.c) or [`SPL/../main.c`](./SPL/Source/main.c) 
