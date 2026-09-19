#include "STD_TYPES.h"
#include "MCAL/DIO/DIO_interface.h"
#include "MCAL/TWI/TWI_interface.h"
#include "MCAL/UART/UART_interface.h"
#include "HAL/LCD/LCD_interface.h"
#include "HAL/KEYPAD/KEYPAD_interface.h"
#include "HAL/EEPROM/EEPROM_interface.h"
#include "HAL/FINGREPRINT/FINGREPRINT_interface.h"

// Hardware Pins
#define BUZZER_PORT  DIO_PORTC
#define BUZZER_PIN   DIO_PIN3

#define LED_GRN_PORT DIO_PORTC
#define LED_GRN_PIN  DIO_PIN4

#define LED_RED_PORT DIO_PORTC
#define LED_RED_PIN  DIO_PIN5

// EEPROM Memory Addresses
#define SLOT_1_ADDR  0x00
#define SLOT_2_ADDR  0x01

// Global Peripheral State Tracking
static u8 g_led_state = 0;
static u8 g_buzzer_state = 0;

// Function Prototypes
void System_Init(void);
void Admin_Mode(void);
void Recog_Mode(void);
void Hardware_Control_Menu(u8 user_id);
void Update_Component_Status_LCD(void);
void Timer1_Delay_ms(u16 delay_ms);
u8 Keypad_GetPressWithTimeout(u16 timeout_ms);
void Sound_SuccessBeep(void);
void Sound_ErrorBeep(void);

// --- TIMER 1 DELAY ROUTINE ---
void Timer1_Delay_ms(u16 delay_ms) {
    for (u16 i = 0; i < delay_ms; i++) {
        TIMER1_voidSetTimerValue(0);
        while (TIMER1_u16ReadimerValue() < 1000); // 1000 us = 1 ms
    }
}

// --- NON-BLOCKING KEYPAD POLLER ---
u8 Keypad_GetPressWithTimeout(u16 timeout_ms) {
    u8 key = 0;
    u16 elapsed = 0;

    while (elapsed < timeout_ms) {
        key = KEYPAD_GetPressed();
        if (key != 0) {
            Timer1_Delay_ms(150); // Debounce delay
            return key;
        }
        Timer1_Delay_ms(10);
        elapsed += 10;
    }
    return 0; // Timed out
}

// --- AUDIO FEEDBACK ROUTINES ---
void Sound_SuccessBeep(void) {
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_HIGH);
    Timer1_Delay_ms(100);
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_LOW);
    Timer1_Delay_ms(50);
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_HIGH);
    Timer1_Delay_ms(150);
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_LOW);
}

void Sound_ErrorBeep(void) {
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_HIGH);
    Timer1_Delay_ms(300);
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_LOW);
}

int main(void) {
    System_Init();
    u8 key = 0;

    while (1) {
        LCD_clear();
        LCD_writeString("1: Admin Mode");
        LCD_goToSpecificPosition(2, 0);
        LCD_writeString("2: Recog Mode");

        key = Keypad_GetPressWithTimeout(3000);

        if (key == 1 || key == '1') {
            Admin_Mode();
        } else if (key == 2 || key == '2') {
            Recog_Mode();
        }
    }
}

void System_Init(void) {
    LCD_init();
    KEYPAD_Init();

    // Activate I2C Pull-Up Resistors for EEPROM (PC0, PC1)
    DIO_SetPinDirection(DIO_PORTC, DIO_PIN0, DIO_PIN_INPUT);
    DIO_SetPinDirection(DIO_PORTC, DIO_PIN1, DIO_PIN_INPUT);
    DIO_ActivePinInPullUpResistance(DIO_PORTC, DIO_PIN0);
    DIO_ActivePinInPullUpResistance(DIO_PORTC, DIO_PIN1);

    TWI_Init();
    UART_Init();
    TIMERS_voidTimer1Init();

    // Set Pin Directions
    DIO_SetPinDirection(BUZZER_PORT, BUZZER_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_OUTPUT);
    DIO_SetPinDirection(LED_RED_PORT, LED_RED_PIN, DIO_PIN_OUTPUT);

    // Initial Output States (OFF)
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_LOW);
    DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_LOW);
    DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_LOW);
}

// --- RECOGNITION MODE ---
void Recog_Mode(void) {
    u8 slot1_status = EEPROM_ReadByte(SLOT_1_ADDR);
    u8 slot2_status = EEPROM_ReadByte(SLOT_2_ADDR);

    if (slot1_status != 1) slot1_status = 0;
    if (slot2_status != 1) slot2_status = 0;

    if (slot1_status == 0 && slot2_status == 0) {
        LCD_clear();
        LCD_writeString("Nothing Added!");
        Sound_ErrorBeep();
        Timer1_Delay_ms(1500);
        return;
    }

    u8 attempts = 3;
    while (attempts > 0) {
        LCD_clear();
        LCD_writeString("Scan Finger...");

        u8 found_id = 0;
                u8 result = STD_TYPES_NOK;

                // Strictly 5-Second Scan Window (10 cycles * ~400ms total UART+Delay = ~4-5 sec)
                for (u8 t = 0; t < 10; t++) {
                    result = FINGERPRINT_Search(&found_id);
                    if (result == STD_TYPES_OK) {
                        break; // Match confirmed immediately
                    }
                    Timer1_Delay_ms(100); // Small 100ms delay between scans
                }

        if (result == STD_TYPES_OK) {
            // Personalized Greeting & Success Feedback
            DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_HIGH);
            LCD_clear();
            LCD_writeString("Welcome User ");
            LCD_writeNumber(found_id);
            LCD_writeString("!");
            LCD_goToSpecificPosition(2, 0);
            LCD_writeString("Access Granted");

            Sound_SuccessBeep();
            Timer1_Delay_ms(1500);
            DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_LOW);

            // Hardware Control Panel Access
            Hardware_Control_Menu(found_id);
            return;
        } else {
            // Failed Attempt
            attempts--;
            DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_HIGH);
            LCD_clear();
            LCD_writeString("Wrong! Tries: ");
            LCD_writeNumber(attempts);

            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_LOW);
        }
    }

    // --- 10-SECOND LOCKOUT ON 3 FAILS ---
    LCD_clear();
    LCD_writeString("System Locked!");
    LCD_goToSpecificPosition(2, 0);
    LCD_writeString("Wait 10 Sec...");

    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_HIGH);
    Timer1_Delay_ms(10000);
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_LOW);
}

// --- HARDWARE COMPONENT STATUS DISPLAY ---
void Update_Component_Status_LCD(void) {
    LCD_clear();
    LCD_writeString("1:LED:");
    LCD_writeString(g_led_state ? "ON " : "OFF");
    LCD_writeString(" 2:BUZ:");
    LCD_writeString(g_buzzer_state ? "ON" : "OFF");
    LCD_goToSpecificPosition(2, 0);
    LCD_writeString("3: Exit Menu");
}

// --- HARDWARE CONTROL MENU ---
// --- HARDWARE CONTROL MENU (NO FLICKER) ---
void Hardware_Control_Menu(u8 user_id) {
    u8 key = 0;
    u16 idle_counter = 0;

    // Render static interface ONCE on entry
    Update_Component_Status_LCD();


//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Time for HARDWARE menu
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

    while (idle_counter < 200) { // 5-second timeout (200 * 100ms)
        key = Keypad_GetPressWithTimeout(100);

        if (key == 1 || key == '1') {
            g_led_state = !g_led_state;
            DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, g_led_state ? DIO_PIN_HIGH : DIO_PIN_LOW);
            Update_Component_Status_LCD(); // Update display ONLY when state changes
            idle_counter = 0;
        } else if (key == 2 || key == '2') {
            g_buzzer_state = !g_buzzer_state;
            DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, g_buzzer_state ? DIO_PIN_HIGH : DIO_PIN_LOW);
            Update_Component_Status_LCD(); // Update display ONLY when state changes
            idle_counter = 0;
        } else if (key == 3 || key == '3') {
            break; // Manual exit button
        }

        idle_counter++;
    }

    // Cleanup states on logout
    DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_LOW);
    DIO_SetPinValue(BUZZER_PORT, BUZZER_PIN, DIO_PIN_LOW);
    g_led_state = 0;
    g_buzzer_state = 0;

    LCD_clear();
    LCD_writeString("Logging Out...");
    Timer1_Delay_ms(800);
}

// --- ADMIN MODE ---
void Admin_Mode(void) {
    LCD_clear();
    LCD_writeString("1:Add 2:Edit");
    LCD_goToSpecificPosition(2, 0);
    LCD_writeString("3:Delete");


//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// Time for the add menu
///////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

    u8 key = Keypad_GetPressWithTimeout(10000);
    if (key == 0) return;

    u8 slot1_status = EEPROM_ReadByte(SLOT_1_ADDR);
    u8 slot2_status = EEPROM_ReadByte(SLOT_2_ADDR);

    if (slot1_status != 1) slot1_status = 0;
    if (slot2_status != 1) slot2_status = 0;

    if (key == '1') key = 1;
    if (key == '2') key = 2;
    if (key == '3') key = 3;

    LCD_clear();

    // ==========================================
    // 1. ADD MODE
    // ==========================================
    if (key == 1) {
        if (slot1_status == 1 && slot2_status == 1) {
            LCD_writeString("No Storage!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        if (slot1_status == 0 && slot2_status == 0) {
            LCD_writeString("Select ID: 1 or 2");
        } else if (slot1_status == 0) {
            LCD_writeString("Add: 1 (2 Full)");
        } else if (slot2_status == 0) {
            LCD_writeString("Add: 2 (1 Full)");
        }

        u8 selected_id = Keypad_GetPressWithTimeout(10000);
        if (selected_id == 0) return;
        if (selected_id == '1') selected_id = 1;
        if (selected_id == '2') selected_id = 2;

        if ((selected_id != 1 && selected_id != 2) ||
            (selected_id == 1 && slot1_status == 1) ||
            (selected_id == 2 && slot2_status == 1)) {
            LCD_clear();
            LCD_writeString("Invalid/Occupied");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        LCD_clear();
        LCD_writeString("Place Finger...");
        LCD_goToSpecificPosition(2, 0);
        LCD_writeString("Waiting (5s)");

        u8 result = STD_TYPES_NOK;
        for (u8 i = 0; i < 20; i++) {
            result = FINGERPRINT_Enroll(selected_id);
            if (result == STD_TYPES_OK) break;
            Timer1_Delay_ms(250);
        }

        LCD_clear();
        if (result == STD_TYPES_OK) {
            DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_HIGH);
            LCD_writeString("Success!");
            Sound_SuccessBeep();
            if (selected_id == 1) EEPROM_WriteByte(SLOT_1_ADDR, 1);
            if (selected_id == 2) EEPROM_WriteByte(SLOT_2_ADDR, 1);
        } else {
            DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_HIGH);
            LCD_writeString("Failed / Timeout");
            Sound_ErrorBeep();
        }

        Timer1_Delay_ms(1500);
        DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_LOW);
        DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_LOW);
    }

    // ==========================================
    // 2. EDIT MODE
    // ==========================================
    else if (key == 2) {
        if (slot1_status == 0 && slot2_status == 0) {
            LCD_writeString("Memory Empty!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        LCD_writeString("Edit ID: 1 or 2");
        u8 id_key = Keypad_GetPressWithTimeout(10000);
        if (id_key == 0) return;
        if (id_key == '1') id_key = 1;
        if (id_key == '2') id_key = 2;

        if (id_key != 1 && id_key != 2) {
            LCD_clear();
            LCD_writeString("Invalid ID!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        // Strict slot check: Cannot edit non-existent ID
        if ((id_key == 1 && slot1_status == 0) || (id_key == 2 && slot2_status == 0)) {
            LCD_clear();
            LCD_writeString("ID ");
            LCD_writeNumber(id_key);
            LCD_writeString(" is Empty!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        LCD_clear();
        LCD_writeString("Place New Finger");
        LCD_goToSpecificPosition(2, 0);
        LCD_writeString("Waiting (5s)");

        u8 result = STD_TYPES_NOK;
        for (u8 i = 0; i < 20; i++) {
            result = FINGERPRINT_Enroll(id_key);
            if (result == STD_TYPES_OK) break;
            Timer1_Delay_ms(250);
        }

        LCD_clear();
        if (result == STD_TYPES_OK) {
            DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_HIGH);
            LCD_writeString("Edit Success!");
            Sound_SuccessBeep();
            if (id_key == 1) EEPROM_WriteByte(SLOT_1_ADDR, 1);
            if (id_key == 2) EEPROM_WriteByte(SLOT_2_ADDR, 1);
        } else {
            DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_HIGH);
            LCD_writeString("Edit Failed!");
            Sound_ErrorBeep();
        }

        Timer1_Delay_ms(1500);
        DIO_SetPinValue(LED_GRN_PORT, LED_GRN_PIN, DIO_PIN_LOW);
        DIO_SetPinValue(LED_RED_PORT, LED_RED_PIN, DIO_PIN_LOW);
    }

    // ==========================================
    // 3. DELETE MODE
    // ==========================================
    else if (key == 3) {
        if (slot1_status == 0 && slot2_status == 0) {
            LCD_writeString("Memory Empty!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        LCD_writeString("Del ID: 1 or 2");
        u8 id_key = Keypad_GetPressWithTimeout(10000);
        if (id_key == 0) return;

        if (id_key == '1') id_key = 1;
        if (id_key == '2') id_key = 2;

        if (id_key != 1 && id_key != 2) {
            LCD_clear();
            LCD_writeString("Invalid ID!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        // Strict slot check: Cannot delete non-existent ID
        if ((id_key == 1 && slot1_status == 0) || (id_key == 2 && slot2_status == 0)) {
            LCD_clear();
            LCD_writeString("Already Empty!");
            Sound_ErrorBeep();
            Timer1_Delay_ms(1500);
            return;
        }

        u8 result = FINGERPRINT_Delete(id_key);
        LCD_clear();

        if (result == STD_TYPES_OK) {
            LCD_writeString("Deleted OK!");
            Sound_SuccessBeep();
            if (id_key == 1) EEPROM_WriteByte(SLOT_1_ADDR, 0);
            if (id_key == 2) EEPROM_WriteByte(SLOT_2_ADDR, 0);
        } else {
            LCD_writeString("Delete Failed!");
            Sound_ErrorBeep();
        }
        Timer1_Delay_ms(1500);
    }
}
