/*
 * eeprom.h
 *
 *  Created on: Sep 2, 2024
 *      Author: BACANCY
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

// Define the I2C address of the M24C04 EEPROM
//#define M24C04_I2C_ADDRESS       0x50 << 1 // STM32 uses 8-bit addressing
#define EEPROM_I2C_ADD1 0X56
#define EEPROM_I2C_ADD2 0X57
#define EEPROM_TOTAL_MEMORY      (512U)

#define EEPROM_SINGLE_PAGE_SIZE  (16U)
#define EEPROM_TOTAL_PAGES       (EEPROM_TOTAL_MEMORY/EEPROM_SINGLE_PAGE_SIZE)

// Function prototypes
HAL_StatusTypeDef rEEpromWrite(uint16_t u16MemAddress, uint8_t *u8Data, uint16_t u16TotalData);
HAL_StatusTypeDef rEEpromRead(uint16_t u16MemAddress, uint8_t *u8Data, uint16_t u16Size);

#ifdef __cplusplus
}
#endif

#endif /* EEPROM_H_ */
