/*
 * TWI_interface.h
 *
 *  Created on: Sep 9, 2026
 *      Author: mosta
 */

#ifndef MCAL_TWI_TWI_INTERFACE_H_
#define MCAL_TWI_TWI_INTERFACE_H_

#include "../../STD_TYPES.h"

void TWI_Init(void);
void TWI_Start(void);
void TWI_Stop(void);
void TWI_Write(u8 Data);
u8 TWI_ReadWithNoAck(void);

#endif /* MCAL_TWI_TWI_INTERFACE_H_ */
