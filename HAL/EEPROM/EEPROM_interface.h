/*
 * EEPROM_interface.h
 *
 *  Created on: Sep 9, 2026
 *      Author: mosta
 */

#ifndef HAL_EEPROM_EEPROM_INTERFACE_H_
#define HAL_EEPROM_EEPROM_INTERFACE_H_

#include "../../STD_TYPES.h"

void EEPROM_WriteByte(u16 Address, u8 Data);
u8 EEPROM_ReadByte(u16 Address);

#endif /* HAL_EEPROM_EEPROM_INTERFACE_H_ */
