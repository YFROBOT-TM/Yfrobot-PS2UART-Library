#ifndef YFPS2UART_HW_H
#define YFPS2UART_HW_H

#include <Arduino.h>

// PS2按键定义
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
#define PSB_GREEN       0x1000
#define PSB_RED         0x2000
#define PSB_BLUE        0x4000
#define PSB_PINK        0x8000
#define PSB_TRIANGLE    0x1000
#define PSB_CIRCLE      0x2000
#define PSB_CROSS       0x4000
#define PSB_SQUARE      0x8000

// PS2摇杆定义
#define PSS_RX 5
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

// 震动命令定义
#define VIBRATE_BOTH 0x01
#define VIBRATE_LEFT 0x02
#define VIBRATE_RIGHT 0x03

class YFPS2UART_HW {
public:
    YFPS2UART_HW();
    
    void begin(unsigned long baud = 9600);
    void update();
    
    bool Button(uint16_t button);
    bool ButtonPressed(uint16_t button);
    bool ButtonReleased(uint16_t button);
    bool NewButtonState();
    bool ButtonChanged(uint16_t button);
    
    uint8_t Analog(byte button);
    
    void sendVibrate(uint8_t cmd);
    
    void setDebounceMs(uint16_t debounceMs);
    uint16_t getDebounceMs() const;
    
    bool isRemoteConnected() const;
    bool hasRecentData(uint32_t timeoutMs = 1000) const;

private:
    HardwareSerial* _serial;
    unsigned long _lastReceiveTime;
    bool _newData;
    byte _buf[8];
    
    bool _ignoreIncoming;
    bool _receiving;
    uint8_t _ndx;
    bool _pendingStart;
    
    uint16_t _rawButtons;
    uint16_t _stableButtons;
    uint32_t _debounceStartMs;
    uint16_t _debounceMs;
    
    uint16_t _lastButtons;
    uint16_t _prevStableButtons;
    uint16_t _pressedEvents;
    uint16_t _releasedEvents;
    uint16_t _changedEvents;
    uint32_t _holdStartMs[16];
    
    uint8_t _leftX, _leftY, _rightX, _rightY;

    void readDataFromSerial();
};

#endif // YFPS2UART_HW_H
