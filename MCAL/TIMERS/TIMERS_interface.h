/*
 * TIMERS_interface.h
 *
 *  Created on: Sep 6, 2026
 *      Author: mosta
 */

#ifndef MCAL_TIMERS_TIMERS_INTERFACE_H_
#define MCAL_TIMERS_TIMERS_INTERFACE_H_




void TIMERS_voidTimer0Init (void);
/******************************TIMER0************************************/


u8 TIMERS_u8Timer0OVFSetCallBack(void(*copy_pf)(void));

u8 TIMERS_u8Timer0CTCSetCallBack(void(*copy_pf)(void));

void TIMERS_voidTimer0SetCompareMatchValue(u8 Copy_OCR0Value);

/******************************TIMER1************************************/

void TIMERS_voidTimer1Init (void);


void TIMERS_voidTimer1SetCompareMatchValue(u16 Copy_OCR1Value);
void TIMER1_voidSetTimerValue(u16 Copy_OCR1Value);
u16 TIMER1_u16ReadimerValue(void);


#endif /* MCAL_TIMERS_TIMERS_INTERFACE_H_ */
