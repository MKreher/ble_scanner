#ifndef NRF52840_HAL_H
#define NRF52840_HAL_H

#include "stdint.h"

#include "sdk_common.h"

#define GPIO_PORT_RST 0
#define GPIO_PORT_DC 0
#define GPIO_PORT_CS 0

#define GPIO_PIN_RST 4 //RST
#define GPIO_PIN_DC 28 //DC
#define GPIO_PIN_CS SPI_SS_PIN //SS

void HAL_Init(void);

void SPI_TXBuffer(uint8_t *buffer, uint32_t len);

void SPI_TXByte(uint8_t data);

void GPIO_SetPin(uint16_t Port, uint16_t Pin);

void GPIO_ResetPin(uint16_t Port, uint16_t Pin);

void HAL_Delay(uint16_t ms);

#endif // NRF52840_HAL_H