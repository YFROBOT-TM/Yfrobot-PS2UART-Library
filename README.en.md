# YFPS2UART Library

## Overview
The YFPS2UART is a powerful Arduino library for connecting PS2 controllers via UART serial communication. This library supports multiple Arduino-compatible boards, including Arduino UNO and ESP32 series, providing rich APIs for reading button states, joystick values, sending vibration commands, and more.

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

### Arduino UNO Example
```cpp
#include <YFPS2UART.h>

// Arduino UNO R3 pin configuration
YFPS2UART ps2uart(11, 10);  // RX, TX

void setup() {
  Serial.begin(115200);
  ps2uart.setDebug(false);
  ps2uart.setDebounceMs(10);  // Set debounce time
  ps2uart.begin(115200);      // Initialize serial communication
  
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

### ESP32 Example
```cpp
#include <YFPS2UART.h>

// ESP32 pin configuration
YFPS2UART ps2uart(16, 17);  // RX, TX

void setup() {
  Serial.begin(115200);
  ps2uart.setDebug(false);
  ps2uart.setDebounceMs(10);  // Set debounce time
  ps2uart.begin(115200);      // Initialize serial communication
  
  Serial.println("YFPS2UART ESP32 example program started");
}

void loop() {
  // Same as Arduino UNO example
}
```

## API Reference

### Constructor
- `YFPS2UART(uint8_t rxPin = 11, uint8_t txPin = 10)`: Creates a YFPS2UART instance with specified RX and TX pins

### Initialization and Configuration
- `void begin(unsigned long espBaud = 115200)`: Initializes the library and sets up serial communication, default baud rate is 115200
- `void setDebug(bool enable)`: Enables/disables debug information output
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
- `bool sendSetBaud(uint32_t baud)`: Sets baud rate (supports 9600 or 115200)
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

## License
This library is licensed under the MIT License. See the LICENSE file in the extras directory for more details.

## Support
For support and contributions, please open an issue in the repository or submit a pull request.