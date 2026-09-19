#include "../../STD_TYPES.h"
#include "../../BIT_MATH.h"

#define TWBR *((volatile u8*)0x20)
#define TWCR *((volatile u8*)0x56)
#define TWSR *((volatile u8*)0x21)
#define TWDR *((volatile u8*)0x23)

void TWI_Init(void) {
    // Set Prescaler to 1, Bit Rate to 400kbps (Assuming 8MHz CPU)
    TWBR = 2;
    CLR_BIT(TWSR, 0);
    CLR_BIT(TWSR, 1);
    // Enable TWI
    SET_BIT(TWCR, 2);
}

void TWI_Start(void) {
    // Clear flag, Enable TWI, send Start condition
    TWCR = (1<<7) | (1<<5) | (1<<2);
    while (GET_BIT(TWCR, 7) == 0); // Wait for flag
}

void TWI_Stop(void) {
    TWCR = (1<<7) | (1<<4) | (1<<2);
}

void TWI_Write(u8 Data) {
    TWDR = Data;
    TWCR = (1<<7) | (1<<2); // Clear flag to send data
    while (GET_BIT(TWCR, 7) == 0); // Wait for flag
}

u8 TWI_ReadWithNoAck(void) {
    TWCR = (1<<7) | (1<<2); // Clear flag to receive data (No ACK)
    while (GET_BIT(TWCR, 7) == 0);
    return TWDR;
}
