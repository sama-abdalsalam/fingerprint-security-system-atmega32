# ATmega32 Fingerprint Security System
A bare-metal, biometric access control system built on the ATmega32 microcontroller. This was developed as the final project for the ITI Embedded Systems internship.

## 🚀 Overview
This project implements a commercial-grade smart lock system. It avoids blocking delays by using hardware timers and features a robust layered software architecture (MCAL, HAL, APP). Instead of storing bulky fingerprint images on a small EEPROM, the system utilizes the sensor's internal flash for biometric data and uses the external EEPROM purely as a non-volatile database index.

## ⚙️ Hardware Components
* **Microcontroller:** ATmega32
* **Biometric Sensor:** R307 Optical Fingerprint Module (UART)
* **Memory:** 24C08 External EEPROM (I2C/TWI)
* **UI:** 16x2 LCD & 4x4 Matrix Keypad
* **Feedback:** LEDs and Alarm Buzzer

## 🏗️ Software Architecture
The codebase is strictly divided into three layers to ensure portability and hardware abstraction:
* **MCAL (Microcontroller Abstraction Layer):** Custom drivers for DIO, UART (Asynchronous), TWI/I2C (Synchronous), and Hardware Timers.
* **HAL (Hardware Abstraction Layer):** Interfaces for the LCD, Keypad, R307 Sensor (packet generation), and EEPROM data handling.
* **APP (Application Layer):** A state machine managing `Admin Mode` (Add/Edit/Delete users) and `Recognition Mode` (Scanning, 3-attempt lockouts, and hardware control payloads).

## 💡 Key Technical Features
* **Zero-Blocking Polling:** Hardware Timer1 is used to create precise scanning timeout windows, preventing the CPU from freezing while waiting for user input.
* **Smart Data Indexing:** The ATmega32 coordinates the UART sensor and I2C EEPROM, ensuring memory flags are only updated upon successful biometric enrollment.
* **Security Lockout:** 3 consecutive failed fingerprint scans trigger a system lockdown and activate a hardware buzzer.
