# PS2UART Library

## Overview
The PS2UART library provides an easy-to-use interface for interfacing with PS2 controllers via UART communication. It allows users to read button states and send vibration commands based on user input.

## Installation
To install the PS2UART library, follow these steps:
1. Download the library from the repository.
2. Extract the contents to your Arduino libraries folder (typically located in `Documents/Arduino/libraries`).
3. Restart the Arduino IDE to recognize the new library.

## Usage
To use the PS2UART library in your Arduino sketch, include the library and create an instance of the `PS2UART` class. Below is a simple example demonstrating how to initialize the library and read button states.

### Example Code
```cpp
#include <PS2UART.h>

PS2UART ps2;

void setup() {
  Serial.begin(9600);
  ps2.begin();
}

void loop() {
  ps2.readData();
  unsigned int buttons = ps2.getButtons();
  // Add logic to handle button presses and send vibration commands
}
```

## API Reference

### PS2UART Class
- **Public Methods**:
  - `void begin()`: Initializes the library and sets up serial communication.
  - `void readData()`: Reads data from the PS2 controller.
  - `unsigned int getButtons()`: Returns the current button states.
  - `void checkAndSendVibrateCommands()`: Checks button states and sends vibration commands accordingly.

## License
This library is licensed under the MIT License. See the `LICENSE` file in the extras directory for more details.

## Support
For support and contributions, please open an issue in the repository or submit a pull request.