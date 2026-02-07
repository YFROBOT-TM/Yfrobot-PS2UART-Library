// YFPS2UART.h
#ifndef YFPS2UART_H
#define YFPS2UART_H

#include <Arduino.h>

#if defined(__AVR__) || defined(ESP8266) || defined(NRF52) || defined(NRF5) 
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#elif defined(ESP32)
#include <HardwareSerial.h>
#endif

// 抽象串口基类
class SerialBase {
public:
    virtual ~SerialBase() {}
    virtual void begin(unsigned long baud) = 0;
    virtual int available() = 0;
    virtual int read() = 0;
    virtual void write(uint8_t data) = 0;
    virtual void print(const char* str) = 0;
    virtual void flush() = 0;
};

// 硬件串口适配器
class HardwareSerialAdapter : public SerialBase {
private:
    HardwareSerial* _serial;
    int8_t _rxPin;
    int8_t _txPin;
public:
    HardwareSerialAdapter(HardwareSerial* serial, int8_t rxPin = -1, int8_t txPin = -1) 
        : _serial(serial), _rxPin(rxPin), _txPin(txPin) {}
    void begin(unsigned long baud) override {
#if defined(ESP32)
        if (_rxPin >= 0 && _txPin >= 0) {
            _serial->begin(baud, SERIAL_8N1, _rxPin, _txPin);
        } else {
            _serial->begin(baud);
        }
#else
        _serial->begin(baud);
#endif
    }
    int available() override {
        return _serial->available();
    }
    int read() override {
        return _serial->read();
    }
    void write(uint8_t data) override {
        _serial->write(data);
    }
    void print(const char* str) override {
        _serial->print(str);
    }
    void flush() override {
        _serial->flush();
    }
};

#if defined(__AVR__) || defined(ESP8266) || defined(NRF52) || defined(NRF5)
// 软件串口适配器
class SoftwareSerialAdapter : public SerialBase {
private:
    SoftwareSerial* _serial;
public:
    SoftwareSerialAdapter(SoftwareSerial* serial) : _serial(serial) {}
    void begin(unsigned long baud) override {
        _serial->begin(baud);
    }
    int available() override {
        return _serial->available();
    }
    int read() override {
        return _serial->read();
    }
    void write(uint8_t data) override {
        _serial->write(data);
    }
    void print(const char* str) override {
        _serial->print(str);
    }
    void flush() override {
        _serial->flush();
    }
};
#endif

enum SerialType {
  SERIALTYPE_SW,
  SERIALTYPE_HW
};
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

// PS2摇杆定义 - 与PS2X库保持一致
#define PSS_RX 5
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

// 震动命令定义
#define VIBRATE_BOTH 0x01
#define VIBRATE_LEFT 0x02
#define VIBRATE_RIGHT 0x03

class YFPS2UART {
public:
    // Constructor
    // 构造函数优化：如果是 AVR，可以选软硬串口；如果是 ESP32，强制硬串口
#if defined(__AVR__) || defined(ESP8266) || defined(NRF52) || defined(NRF5) 
    YFPS2UART(SerialType serialType = SERIALTYPE_SW, uint8_t rxPin = 11, uint8_t txPin = 10, HardwareSerial* hwSerial = &Serial);
#elif defined(ESP32) 
    YFPS2UART(uint8_t rxPin = 16, uint8_t txPin = 17, HardwareSerial* hwSerial = &Serial2);
#endif
    ~YFPS2UART();

    // Public methods

    void begin(unsigned long espBaud = 9600);
    void update();  // 在 loop 中定期调用，处理接收数据并触发震动检测

    // 去抖设置 & 读取按键，可配置的去抖时间（ms）
    void setDebounceMs(uint16_t ms);
    unsigned int getButtons();        // 去抖后的稳定按键值
    unsigned int getRawButtons();     // 最近帧原始按键值（未去抖）
    
    // 按键事件查询：边沿/释放/按住
    // wasPressed/wasReleased 返回边沿事件（调用后清除对应事件位）
    // bool wasPressed(uint16_t mask);      // 某位刚被按下（edge）
    // bool wasReleased(uint16_t mask);     // 某位刚被释放（edge）
    
    // 参考PS2X库添加的函数
    // bool NewButtonState();               // 检查是否有任何按键状态改变
    // bool NewButtonState(uint16_t button); // 检查特定按键是否有状态改变
    bool Button(uint16_t button);        // 检查按键当前是否被按下
    bool ButtonPressed(uint16_t button);  // 检查按键是否刚被按下
    bool ButtonReleased(uint16_t button); // 检查按键是否刚被释放
    
    
    // 获取摇杆值（0-255）
    uint8_t Analog(byte axis);
    
    // 手动发送震动命令
    void sendVibrate(uint8_t cmd);

    // 新增：向对端发送任意 AT 指令（会发送 CR+LF）
    void sendATCommand(const char *cmd);

    // 新增：发送软件复位指令 AT+RST
    void sendResetCommand();

    // 新增：发送修改波特率指令 AT+BAUD=<baud>
    // 返回 true 表示命令已发送（仅支持 9600 / 115200），
    bool sendSetBaud(uint32_t baud);

    // 新增：发送 AT 指令并等待响应，带超时，响应写入 respBuf（包含末尾 NUL）。
    // 返回 true 表示在超时内收到响应行（以 '\\n' 结束），respBuf 包含响应文本（已去掉末尾 CR/LF）。
    bool sendATCommandWithResponse(const char *cmd, char *respBuf, size_t bufLen, uint32_t timeoutMs = 500);

    // 新增：查询当前波特率
    bool queryBaudRate(uint32_t& baudRate, uint32_t timeoutMs = 500);
    
    // 新增：发送 AT 指令并把响应直接打印到主串口（Serial），返回是否收到响应
    bool sendATCommandPrintResponse(const char *cmd, uint32_t timeoutMs = 500);

    // 新增：远端是否处于已连接（非 0xAB 忽略模式）
    bool isRemoteConnected() const;
    bool hasRecentData(uint32_t timeoutMs = 1000) const;

private:

    SerialBase* _serial;         // 统一指向当前使用的串口对象
#if defined(__AVR__) || defined(ESP8266) || defined(NRF52) || defined(NRF5) 
    SoftwareSerial* _sw;     // 仅在软串口模式下分配内存
    HardwareSerial* _hw;
    SerialType _serialType;
    uint8_t _rxPin, _txPin;
#elif defined(ESP32)
    HardwareSerial* _hw;
    uint8_t _rxPin, _txPin;
    SerialType _serialType;
#endif
    unsigned long _lastReceiveTime;
    bool _newData;
    byte _buf[8];
    
    bool _ignoreIncoming;    // 当收到 0xAB（表示手柄未连接）时，库会忽略后续的协议数据，直到下一个 0x0D 起始字节到来
    // 新增：将接收状态从函数静态变量移到成员，方便外部检查/控制
    bool _receiving;      // 是否正在接收一个帧（遇到 start_MA 后为 true）
    uint8_t _ndx;         // 当前写入 _buf 的索引
    bool _pendingStart;   // 已接收到 start_MA，但尚未交由 readDataFromSerial 处理（避免丢失）
    
    // unsigned int lastButtons;
    // unsigned long lastReceiveTime;
    // byte Read_Bytes[8];
    // boolean newData;

    // 去抖 / 按键存储
    uint16_t _rawButtons;      // 最近一帧解析出的原始按键值
    uint16_t _stableButtons;   // 去抖后对外返回的按键值
    uint32_t _debounceStartMs; // 去抖开始时间
    uint16_t _debounceMs;      // 去抖阈值（毫秒）
    // 按键事件检测 按下 按住 释放
    uint16_t _lastButtons;
    uint16_t _prevStableButtons;   // 上一周期稳定按键（用于边沿检测）
    uint16_t _pressedEvents;      // 记录未读的按下事件（bit）
    uint16_t _releasedEvents;     // 记录未读的释放事件（bit）
    uint16_t _changedEvents;      // 按键状态变化事件标志位（用于NewButtonState）
    uint32_t _holdStartMs[16];    // 每位按键的按住开始时间（0 表示未按）
    
    // 摇杆缓存
    uint8_t _leftX, _leftY, _rightX, _rightY;

    void readDataFromSerial();
};

#endif // YFPS2UART_H