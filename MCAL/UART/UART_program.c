#include "../../STD_TYPES.h"
#include "../../BIT_MATH.h"

#define UDR   *((volatile u8*)0x2C)
#define UCSRA *((volatile u8*)0x2B)
#define UCSRB *((volatile u8*)0x2A)
#define UCSRC *((volatile u8*)0x40)
#define UBRRL *((volatile u8*)0x29)
#define UBRRH *((volatile u8*)0x40)

void UART_Init(void) {
    // Set Baud Rate to 57600 (assuming 8MHz CPU)
    u8 UCSRC_Value = 0b10000000;
    UBRRH = 0;
    UBRRL = 8; // 8 for 57600 @ 8MHz

    // 8 Data bits, No Parity, 1 Stop bit
    SET_BIT(UCSRC_Value, 1);
    SET_BIT(UCSRC_Value, 2);
    UCSRC = UCSRC_Value;

    // Enable Transmitter and Receiver
    SET_BIT(UCSRB, 3);
    SET_BIT(UCSRB, 4);
}

void UART_SendByte(u8 Data) {
    // Wait until transmit buffer is empty
    while (GET_BIT(UCSRA, 5) == 0);
    UDR = Data;
}

u8 UART_ReceiveByte(void) {
    // Wait until data is received
    while (GET_BIT(UCSRA, 7) == 0);
    return UDR;
}
