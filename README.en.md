# YFPS2UART Library[![中文版本](https://img.shields.io/badge/-中文-red)](README.md)

## Language Switch
- [English](README.en.md)
- [中文 (Chinese)](README.md)

## Overview
The YFPS2UART is a powerful Arduino library for connecting PS2 controllers via UART serial communication. This library supports multiple Arduino-compatible boards, including Arduino UNO and ESP32 series, providing rich APIs for reading button states, joystick values, sending vibration commands, and more.

## Product Purchase Links
- Official Website: [https://www.yfrobot.com/](https://www.yfrobot.com/)
- Taobao Store: [https://yfrobot.taobao.com/](https://yfrobot.taobao.com/)

## Features
- Supports Arduino UNO and ESP32 series boards
- Automatic controller connection detection
- Button state reading (pressed/released/hold)
- Joystick analog value reading (X/Y axes)
- Dual vibration motor control
- Built-in button debouncing
- AT command communication support (version query, baud rate modification, etc.)
- Memory-optimized design for resource-limited platforms

## Installation
### Method 1: Install via Arduino IDE Library Manager
1. Open Arduino IDE
2. Click "Tools" > "Manage Libraries..."
3. Type "YFPS2UART" in the search box
4. Click the "Install" button

### Method 2: Manual Installation
1. Download the ZIP file of this library
2. Extract it to the Arduino libraries folder (usually located at `Documents/Arduino/libraries`)
3. Restart Arduino IDE to recognize the new library

## Hardware Connection

### Receiver & Controller Indicator Lights
- Both the receiver and controller have two indicator lights: red and green
- Red light: Power indicator (always on when powered)
- Green light: Connection status indicator
  - Blinking: Not connected or pairing in progress
  - Solid: Successfully paired and connected

### Response Times & Timing Information
- **Vibration Duration**: 0.5 seconds per vibration command
- **Controller Pairing Window**: Approximately 2 seconds
- **Initialization Time**: The receiver module initializes within 1-2 seconds after power-on
- **Controller Sleep Timer**:
  - When not connected: Sleeps after 15 seconds of inactivity
  - When connected: Sleeps after 2 minutes of inactivity
- **Controller Pairing Behavior**:
  - If not powered off: Remembers the last paired receiver and automatically reconnects when waking up with START button
  - If powered off: Loses pairing memory and will pair with any available receiver

### Arduino UNO R3 Connection

### Arduino UNO R3 Connection
- Controller module RX connects to Arduino TX (10)
- Controller module TX connects to Arduino RX (11)
- Controller module VCC connects to Arduino 5V
- Controller module GND connects to Arduino GND

### ESP32 Connection
- Controller module RX connects to ESP32 TX (17)
- Controller module TX connects to ESP32 RX (16)
- Controller module VCC connects to ESP32 5V
- Controller module GND connects to ESP32 GND

## Basic Usage

### Arduino UNO Example (Software Serial)
```cpp
#include <YFPS2UART.h>

// Arduino UNO R3 pin configuration, using software serial
YFPS2UART ps2uart(SERIALTYPE_SW, 11, 10);  // SerialType, RX, TX

void setup() {
  Serial.begin(115200);
  ps2uart.setDebounceMs(10);  // Set debounce time
  ps2uart.begin(9600);      // Initialize serial communication, soft serial please use 9600 baud rate
  
  Serial.println("YFPS2UART example program started");
}

void loop() {
  // Update controller data
  ps2uart.update();
  
  // Check controller connection status
  if (!ps2uart.isRemoteConnected()) {
    Serial.println(F("Waiting for controller connection..."));
    delay(1000);
    return;
  }
  
  // Read button states
  if (ps2uart.Button(PSB_START)) {
    Serial.println(F("Start button pressed"));
  }
  
  // Read joystick values
  Serial.print(F("Joystick values: LY="));
  Serial.print(ps2uart.Analog(PSS_LY));
  Serial.print(F(", LX="));
  Serial.print(ps2uart.Analog(PSS_LX));
  Serial.print(F(", RY="));
  Serial.print(ps2uart.Analog(PSS_RY));
  Serial.print(F(", RX="));
  Serial.println(ps2uart.Analog(PSS_RX));
  
  delay(50);
}
```

### Arduino UNO Example (Hardware Serial)
```cpp
#include <YFPS2UART.h>

// Arduino UNO R3 pin configuration, using hardware serial
YFPS2UART ps2uart(SERIALTYPE_HW, 11, 10, &Serial);  // SerialType, RX, TX, HardwareSerial*

void setup() {
  Serial.begin(115200);
  ps2uart.setDebounceMs(10);  // Set debounce time
  ps2uart.begin(9600);      // Initialize serial communication
  
  Serial.println("YFPS2UART example program started");
}

void loop() {
  // Same as software serial example
}
```

### ESP32 Example
```cpp
#include <YFPS2UART.h>

// ESP32 pin configuration, using hardware serial
YFPS2UART ps2uart(16, 17);  // RX, TX

void setup() {
  Serial.begin(115200);
  ps2uart.setDebounceMs(10);  // Set debounce time
  ps2uart.begin(9600);      // Initialize serial communication
  
  Serial.println("YFPS2UART ESP32 example program started");
}

void loop() {
  // Same as Arduino UNO example
}
```

## API Reference

### Constructor

**Arduino UNO (AVR):**
- `YFPS2UART(SerialType serialType = SERIALTYPE_SW, uint8_t rxPin = 11, uint8_t txPin = 10, HardwareSerial* hwSerial = &Serial)`: Creates a YFPS2UART instance
  - `serialType`: Serial type, `SERIALTYPE_SW` (software serial) or `SERIALTYPE_HW` (hardware serial)
  - `rxPin`: RX pin number
  - `txPin`: TX pin number
  - `hwSerial`: Hardware serial pointer (only used when `serialType` is `SERIALTYPE_HW`)

**ESP32:**
- `YFPS2UART(uint8_t rxPin = 16, uint8_t txPin = 17, HardwareSerial* hwSerial = &Serial2)`: Creates a YFPS2UART instance
  - `rxPin`: RX pin number
  - `txPin`: TX pin number
  - `hwSerial`: Hardware serial pointer (defaults to Serial2)

### Initialization and Configuration
- `void begin(unsigned long espBaud = 9600)`: Initializes the library and sets up serial communication, default baud rate is 9600.
- `void setDebounceMs(uint16_t ms)`: Sets button debounce time (milliseconds)

### Data Update and Connection Status
- `void update()`: Updates controller data, should be called regularly in loop()
- `bool isRemoteConnected() const`: Checks if a controller is connected
- `bool hasRecentData(uint32_t timeoutMs = 1000) const`: Checks if there's recent data update

### Button State Query
- `unsigned int getButtons()`: Returns debounced stable button values
- `unsigned int getRawButtons()`: Returns raw button values without debouncing
- `bool Button(uint16_t button)`: Checks if specified button is being held
- `bool ButtonPressed(uint16_t button)`: Checks if specified button was just pressed (edge detection)
- `bool ButtonReleased(uint16_t button)`: Checks if specified button was just released (edge detection)

### Joystick Value Reading
- `uint8_t Analog(byte axis)`: Returns analog value (0-255) for specified joystick axis
  - PSS_LY: Left joystick Y axis
  - PSS_LX: Left joystick X axis
  - PSS_RY: Right joystick Y axis
  - PSS_RX: Right joystick X axis

### Vibration Control
- `void sendVibrate(uint8_t cmd)`: Sends vibration command
  - VIBRATE_BOTH: Both motors vibrate
  - VIBRATE_LEFT: Left motor vibrates
  - VIBRATE_RIGHT: Right motor vibrates

### AT Commands
- `void sendATCommand(const char *cmd)`: Sends AT command
- `void sendResetCommand()`: Sends software reset command
- `bool sendSetBaud(uint32_t baud)`: Sets baud rate (supports 9600, 19200, 38400, 57600, 115200)
- `bool sendATCommandWithResponse(const char *cmd, char *respBuf, size_t bufLen, uint32_t timeoutMs = 500)`: Sends command and reads response
- `bool queryBaudRate(uint32_t& baudRate, uint32_t timeoutMs = 500)`: Queries current baud rate

## Button Definitions
```cpp
#define PSB_SELECT      0x0001
#define PSB_L3          0x0002
#define PSB_R3          0x0004
#define PSB_START       0x0008
#define PSB_PAD_UP      0x0010
#define PSB_PAD_RIGHT   0x0020
#define PSB_PAD_DOWN    0x0040
#define PSB_PAD_LEFT    0x0080
#define PSB_L2          0x0100
#define PSB_R2          0x0200
#define PSB_L1          0x0400
#define PSB_R1          0x0800
#define PSB_TRIANGLE    0x1000
#define PSB_CIRCLE      0x2000
#define PSB_CROSS       0x4000
#define PSB_SQUARE      0x8000
```

## Example Programs
The library includes the following example programs:
- `YFPS2UART_Demo`: Basic functionality demonstration including button and joystick reading
- `YFPS2UART_Demo_ChangeBAUD`: Baud rate modification example
- `YFPS2UART_ESP_Demo`: ESP32 platform-specific example
- `YFPS2UART_ESP_Demo_ChangeBAUD`: ESP32 platform baud rate modification example

## Memory Optimization
This library is optimized for resource-limited platforms (such as Arduino UNO):
- Uses F() macro to store strings in Flash instead of RAM
- Avoids using String class to reduce dynamic memory allocation
- Minimizes buffer sizes
- Removes unused member variables

## Troubleshooting
1. **Connection Issues**: Ensure RX/TX pins are correctly connected and baud rates match
2. **Data Not Updating**: Check if controller is paired and module power is normal
3. **Memory Insufficiency**: Use F() macro to store strings, reduce unnecessary variables
4. **Button Jitter**: Adjust setDebounceMs() parameter

## Update Log
- 2.0.0: Added hardware serial support to resolve conflicts between software serial library and servo library. Usage differs from version 1.X (2026-02-06)
- 1.0.1: Fix UNO soft serial baud rate issue, default baud rate changed to 9600
- 1.0.0: Initial version

## License
This library is licensed under the MIT License. See the LICENSE file in the extras directory for more details.

## Support
For support and contributions, please open an issue in the repository or submit a pull request.