/*

 * KEYPAD_config.h
 *
 *  Created on: Aug 30, 2026
 *      Author: mosta
 */

#ifndef HAL_KEYPAD_KEYPAD_CONFIG_H_
#define HAL_KEYPAD_KEYPAD_CONFIG_H_


extern const  u8 KPD_Switch [4][4] ;       /* 0 0 0 0
 	 	 	 	 	 	 	 	 	 	 	 1 1 1 1
 	 	 	 	 	 	 	 	 	 	 	 2 2  2 2
 	 	 	 	 	 	 	 	 	 	 	 3	3 3 3
 	 	 	 	 	 	 	 	 	 	 	  */





#define  KEYPAD_PORT 			DIO_PORTA


#define  KEYPAD_COLUMN_START 	DIO_PIN4
#define  KEYPAD_COLUMN_END   	DIO_PIN7


#define  KEYPAD_ROW_START 	DIO_PIN0
#define  KEYPAD_ROW_END   	DIO_PIN3





#endif /* HAL_KEYPAD_KEYPAD_CONFIG_H_ */
