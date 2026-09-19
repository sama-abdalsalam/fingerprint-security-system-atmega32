#include "../../STD_TYPES.h"
#include "../../MCAL/UART/UART_interface.h"

// Command to check if sensor is communicating (Verify Password)
static u8 FINGERPRINT_HandshakeCmd[16] = {
    0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00,
    0x07, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B
};

u8 FINGERPRINT_CheckConnection(void) {
    u8 response[16];
    u8 i;

    // 1. Send the command packet to the sensor
    for (i = 0; i < 16; i++) {
        UART_SendByte(FINGERPRINT_HandshakeCmd[i]);
    }

    // 2. Wait and read the reply packet (12 bytes expected for confirmation)
    for (i = 0; i < 12; i++) {
        response[i] = UART_ReceiveByte();
    }

    // 3. Byte index 9 (the 10th byte) is the confirmation code.
    // 0x00 means Success/Sensor found.
    if (response[9] == 0x00) {
        return STD_TYPES_OK;
    } else {
        return STD_TYPES_NOK;
    }
}
#include <util/delay.h>

// --- HELPER FUNCTION: Sends a command and reads the reply ---
static u8 FINGERPRINT_SendCommand(u8* cmd, u8 cmd_len, u8 expected_reply_len, u8* reply_buffer) {
    // 1. Send the command packet
    for (u8 i = 0; i < cmd_len; i++) {
        UART_SendByte(cmd[i]);
    }

    // 2. Receive the reply packet
    for (u8 i = 0; i < expected_reply_len; i++) {
        reply_buffer[i] = UART_ReceiveByte();
    }

    // 3. Return the Confirmation Code (Always at index 9)
    return reply_buffer[9];
}

// --- STANDARD HEX COMMANDS ---
u8 CMD_GenImg[12]   = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05};
u8 CMD_Img2Tz_1[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x08};
u8 CMD_Img2Tz_2[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x02, 0x00, 0x09};
u8 CMD_RegModel[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05, 0x00, 0x09};

// --- API FUNCTIONS ---

u8 FINGERPRINT_Enroll(u8 page_id) {
    u8 reply[12];
    u8 confCode = 1;

    // Step 1: Wait for finger to be placed (Keep looping until success)
    while (confCode != 0x00) {
        confCode = FINGERPRINT_SendCommand(CMD_GenImg, 12, 12, reply);
    }

    // Step 2: Generate Feature File in Buffer 1
    confCode = FINGERPRINT_SendCommand(CMD_Img2Tz_1, 13, 12, reply);
    if (confCode != 0x00) return STD_TYPES_NOK;

    // Give the user 1.5 seconds to lift their finger and put it back down
    _delay_ms(1500);

    // Step 3: Wait for finger to be placed AGAIN (Second Scan)
    confCode = 1;
    while (confCode != 0x00) {
        confCode = FINGERPRINT_SendCommand(CMD_GenImg, 12, 12, reply);
    }

    // Step 4: Generate Feature File in Buffer 2
    confCode = FINGERPRINT_SendCommand(CMD_Img2Tz_2, 13, 12, reply);
    if (confCode != 0x00) return STD_TYPES_NOK;

    // Step 5: Combine both buffers into a high-quality model
    confCode = FINGERPRINT_SendCommand(CMD_RegModel, 12, 12, reply);
    if (confCode != 0x00) return STD_TYPES_NOK;

    // Step 6: Store in Flash Memory (Dynamic Checksum Calculation)
    u16 sum = 0x01 + 0x06 + 0x06 + 0x01 + 0x00 + page_id;
    u8 CMD_Store[15] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06, 0x01, 0x00, page_id, (u8)(sum>>8), (u8)sum};

    confCode = FINGERPRINT_SendCommand(CMD_Store, 15, 12, reply);

    if (confCode == 0x00) return STD_TYPES_OK;
    return STD_TYPES_NOK;
}

u8 FINGERPRINT_Search(u8* found_id) {
    u8 reply[16];
    u8 confCode = 1;
    u16 timeout_counter = 0;

    // Wait up to ~5 seconds for the user to place their finger
    while (confCode != 0x00) {
        confCode = FINGERPRINT_SendCommand(CMD_GenImg, 12, 12, reply);
        _delay_ms(50); // Check every 50ms
        timeout_counter++;

        if (timeout_counter > 100) { // 100 * 50ms = 5 seconds timeout
            return STD_TYPES_NOK; // Timeout: No finger placed
        }
    }

    // 2. Extract features to Buffer 1
    confCode = FINGERPRINT_SendCommand(CMD_Img2Tz_1, 13, 12, reply);
    if (confCode != 0x00) return STD_TYPES_NOK;

    // 3. Search the Database
    u8 CMD_Search[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x18};
    confCode = FINGERPRINT_SendCommand(CMD_Search, 17, 16, reply);

    if (confCode == 0x00) {
        *found_id = reply[11];
        return STD_TYPES_OK;
    }

    return STD_TYPES_NOK; // Finger didn't match
}

u8 FINGERPRINT_Delete(u8 page_id) {
     u8 reply[12];
     // Dynamic Checksum for Delete
     u16 sum = 0x01 + 0x07 + 0x0C + 0x00 + page_id + 0x00 + 0x01;
     u8 CMD_Delete[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, 0x00, page_id, 0x00, 0x01, (u8)(sum>>8), (u8)sum};

     u8 confCode = FINGERPRINT_SendCommand(CMD_Delete, 16, 12, reply);
     if(confCode == 0x00) return STD_TYPES_OK;
     return STD_TYPES_NOK;
}
