/*


* LCD_interface.h
 *
 *  Created on: Aug 27, 2026
 *      Author: mosta
 */

#ifndef HAL_LCD_LCD_INTERFACE_H_
#define HAL_LCD_LCD_INTERFACE_H_


#define LCD_8_BIT_MODE		1
#define LCD_4_BIT_MODE 		2



/*MACROS FOR LCD LINE ID*/


#define LCD_LINE_ONE		1
#define LCD_LINE_TWO		2



/*MACROS FOR LCD SHIFTING DIRECTION*/

#define LCD_SHIFT_LEFT 		0
#define LCD_SHIFT_RIGHT 	1



/*LCD FUNCTION PROTOTYPE*/



void LCD_init(void);

void LCD_sendcmnd( u8 cmnd);

void LCD_sendChar (u8 data);

void writeHalfPort(u8 Value);

void LCD_writeString(u8 * string);


void LCD_writeNumber( u32 number);

void LCD_clear(void);

void LCD_shift(u8 shiftingdirection);

void LCD_goToSpecificPosition (u8 LineNumber , u8 Position);
















#endif /* HAL_LCD_LCD_INTERFACE_H_ */
