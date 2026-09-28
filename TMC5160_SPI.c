/*
MIT License

Copyright (c) 2016 Mike Estee
Copyright (c) 2026 Daniel Sasik

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

#include "TMC5160_SPI.h"
#include "TMC5160_registers.h"

#include "usart.h"
#include "spi.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_spi.h"
#include <stdio.h>

TMC5160_SPI TMC_SPI;

void init_TMC5160_SPI(TMC5160_SPI *TMC_SPI,uint16_t chipSelect, GPIO_TypeDef *GPIO_TYPE)
{
	if (TMC_SPI == NULL)
	{
		printf("init_TMC5160_SPI - TMC_SPI isn't allocated.");
		return;
	}
	TMC_SPI->chipSelectPin = chipSelect;
	TMC_SPI->GPIO_Port = GPIO_TYPE;
}

uint32_t readRegister(uint8_t address)
{
	uint8_t txData[5] = {0};
	uint8_t rxData[5] = {0};
	uint32_t value = 0;

	// Read command
	txData[0] = address & 0x7F;
	// Pull CS low
	HAL_GPIO_WritePin(TMC_SPI.GPIO_Port, TMC_SPI.chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, txData, rxData, 5, HAL_MAX_DELAY);
	// Pull CS high
	HAL_GPIO_WritePin(TMC_SPI.GPIO_Port, TMC_SPI.chipSelectPin, GPIO_PIN_SET);
	value = (rxData[1] << 24) | (rxData[2] << 16) | (rxData[3] << 8) | rxData[4];
	return value;
}

uint8_t writeRegister(uint8_t address, uint32_t data)
{
	uint8_t txData[5] = {0};
	uint8_t rxData[5] = {0};
	uint32_t value = 0;

	// Write command
	txData[0] = address | 0x80;
	txData[1] = (data >> 24) & 0xFF;
	txData[2] = (data >> 16) & 0xFF;
	txData[3] = (data >> 8) & 0xFF;
	txData[4] = data & 0xFF;
	// Pull CS low
	HAL_GPIO_WritePin(TMC_SPI.GPIO_Port, TMC_SPI.chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, txData, rxData, 5, HAL_MAX_DELAY);
	// Pull CS high
	HAL_GPIO_WritePin(TMC_SPI.GPIO_Port, TMC_SPI.chipSelectPin, GPIO_PIN_SET);
	return value;
}

uint8_t readStatus()
{
	uint8_t status = GCONF;
	uint8_t txData[5] = {0};
	uint8_t rxData[5] = {0};
	uint32_t value = 0;

	// Read command
	txData[0] = status & 0x7F;
	// Pull CS low
	HAL_GPIO_WritePin(TMC_SPI.GPIO_Port, TMC_SPI.chipSelectPin, GPIO_PIN_RESET);
	HAL_SPI_TransmitReceive(&hspi2, txData, rxData, 5, HAL_MAX_DELAY);
	// Pull CS high
	HAL_GPIO_WritePin(TMC_SPI.GPIO_Port, TMC_SPI.chipSelectPin, GPIO_PIN_SET);

	value = (rxData[1] << 24) | (rxData[2] << 16) | (rxData[3] << 8) | rxData[4];
	return value;

}
