/*
MIT License

Copyright (c) 2016 Mike Estee (Estee_TMC5160)
Copyright (c) 2017 Tom Magnier

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef TMC5160_SPI_H
#define TMC5160_SPI_H

#include "stm32f4xx_hal.h"

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

typedef struct
{
	uint16_t chipSelectPin;
	GPIO_TypeDef *GPIO_Port;
	uint32_t fclk;
}TMC5160_SPI;

void init_TMC5160_SPI(TMC5160_SPI *TMC_SPI,uint16_t chipSelect, GPIO_TypeDef *GPIO_TYPE);
uint32_t readRegister(uint8_t address);
uint8_t writeRegister(uint8_t address, uint32_t data);
uint8_t readStatus();


#endif
