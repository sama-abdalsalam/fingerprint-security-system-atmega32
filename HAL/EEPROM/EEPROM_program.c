#include "../../STD_TYPES.h"
#include "../../MCAL/TWI/TWI_interface.h"
#include <util/delay.h>

#define EEPROM_BASE_ADDRESS 0xA0

void EEPROM_WriteByte(u16 Address, u8 Data) {
    // Extract the block bits (bits 8 and 9) to put in the device address
    u8 SlaveAddress = EEPROM_BASE_ADDRESS | ((Address >> 7) & 0x06);

    TWI_Start();
    TWI_Write(SlaveAddress);         // Send Slave address with Write (bit 0 = 0)
    TWI_Write((u8)Address);          // Send 8-bit memory location
    TWI_Write(Data);                 // Send the data
    TWI_Stop();
    _delay_ms(5);                    // 5ms delay required by EEPROM to write internally
}

u8 EEPROM_ReadByte(u16 Address) {
    u8 Data = 0;
    u8 SlaveAddress = EEPROM_BASE_ADDRESS | ((Address >> 7) & 0x06);

    TWI_Start();
    TWI_Write(SlaveAddress);         // Slave address with Write
    TWI_Write((u8)Address);          // Memory location
    TWI_Start();                     // Repeated Start
    TWI_Write(SlaveAddress | 0x01);  // Slave address with Read (bit 0 = 1)
    Data = TWI_ReadWithNoAck();      // Read byte
    TWI_Stop();

    return Data;
}
