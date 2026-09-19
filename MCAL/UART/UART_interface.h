/*
 * UART_interface.h
 *
 *  Created on: Sep 9, 2026
 *      Author: mosta
 */

#ifndef MCAL_UART_UART_INTERFACE_H_
#define MCAL_UART_UART_INTERFACE_H_

#include "../../STD_TYPES.h"

void UART_Init(void);
void UART_SendByte(u8 Data);
u8 UART_ReceiveByte(void);

#endif /* MCAL_UART_UART_INTERFACE_H_ */
