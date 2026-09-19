/*



 * LCD_program.c
 *
 *  Created on: Aug 27, 2026
 *      Author: mosta
 */


#include <util/delay.h>

#include "../../STD_TYPES.h"

#include "../../BIT_MATH.h"

#include "../../MCAL/DIO/DIO_interface.h"

#include "../../MCAL/DIO/DIO_private.h"


#include "LCD_config.h"
#include "LCD_interface.h"

static void LCD_sendEnablePulse(void) {
    DIO_SetPinValue(LCD_EN_PORT, LCD_EN_PIN, DIO_PIN_HIGH);
    _delay_us(1);
    DIO_SetPinValue(LCD_EN_PORT, LCD_EN_PIN, DIO_PIN_LOW);
    _delay_us(100);
}

static void LCD_send4Bits(u8 data) {
    DIO_SetPinValue(LCD_D4_PORT, LCD_D4_PIN, READ_BIT(data, 0));
    DIO_SetPinValue(LCD_D5_PORT, LCD_D5_PIN, READ_BIT(data, 1));
    DIO_SetPinValue(LCD_D6_PORT, LCD_D6_PIN, READ_BIT(data, 2));
    DIO_SetPinValue(LCD_D7_PORT, LCD_D7_PIN, READ_BIT(data, 3));
}

void LCD_sendCmd(u8 cmd) {
    DIO_SetPinValue(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_LOW);
    DIO_SetPinValue(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_LOW);

    LCD_send4Bits(cmd >> 4);
    LCD_sendEnablePulse();

    LCD_send4Bits(cmd & 0x0F);
    LCD_sendEnablePulse();
    _delay_ms(2);
}

void LCD_sendChar(u8 data) {
    DIO_SetPinValue(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_HIGH);
    DIO_SetPinValue(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_LOW);

    LCD_send4Bits(data >> 4);
    LCD_sendEnablePulse();

    LCD_send4Bits(data & 0x0F);
    LCD_sendEnablePulse();
    _delay_ms(2);
}

void LCD_init(void) {
    DIO_SetPinDirection(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LCD_EN_PORT, LCD_EN_PIN, DIO_PIN_OUTPUT);

    DIO_SetPinDirection(LCD_D4_PORT, LCD_D4_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LCD_D5_PORT, LCD_D5_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LCD_D6_PORT, LCD_D6_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LCD_D7_PORT, LCD_D7_PIN, DIO_PIN_OUTPUT);

    _delay_ms(50);

    // Initial 4-bit initialization commands
    DIO_SetPinValue(LCD_RS_PORT, LCD_RS_PIN, DIO_PIN_LOW);
    DIO_SetPinValue(LCD_RW_PORT, LCD_RW_PIN, DIO_PIN_LOW);

    LCD_send4Bits(0x02);
    LCD_sendEnablePulse();

    LCD_sendCmd(0x28); // 4-bit mode, 2 lines, 5x7 font
    LCD_sendCmd(0x0C); // Display ON, Cursor OFF
    LCD_sendCmd(0x01); // Clear display
    _delay_ms(2);
    LCD_sendCmd(0x06); // Entry mode set
}

void LCD_clear(void) {
    LCD_sendCmd(0x01);
    _delay_ms(2);
}

void LCD_writeString(u8 *str) {
    while (*str) {
        LCD_sendChar(*str++);
    }
}

void LCD_goToSpecificPosition(u8 line, u8 pos) {
    u8 addr = (line == 1 || line == 0) ? (0x80 + pos) : (0xC0 + pos);
    LCD_sendCmd(addr);
}
void LCD_writeNumber(u32 number) {
    u8 str[10];
    s16 i = 0;

    if (number == 0) {
        LCD_sendChar('0');
        return;
    }

    // Extract digits
    while (number > 0) {
        str[i] = (number % 10) + '0';
        number /= 10;
        i++;
    }
    i--;

    // Print digits in reverse order
    while (i >= 0) {
        LCD_sendChar(str[i]);
        i--;
    }
}
