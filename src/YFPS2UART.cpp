#include "YFPS2UART.h"

// 构造与析构
#if defined(__AVR__) 
YFPS2UART::YFPS2UART(SerialType serialType, uint8_t rxPin, uint8_t txPin, HardwareSerial* hwSerial)
  : _serialType(serialType), _rxPin(rxPin), _txPin(txPin),
    _lastReceiveTime(0), _newData(false),
    _ignoreIncoming(false),
    _receiving(false), _ndx(0), _pendingStart(false),
    _rawButtons(0), _stableButtons(0),
    _prevStableButtons(0), _pressedEvents(0), _releasedEvents(0),
    _changedEvents(0), _lastButtons(0),
    _debounceStartMs(0), _debounceMs(30),
    _leftX(128), _leftY(127), _rightX(128), _rightY(127),
    _sw(nullptr), _hw(hwSerial)
{
  if (_serialType == SERIALTYPE_SW) {
    _sw = new SoftwareSerial(_rxPin, _txPin);
    _serial = new SoftwareSerialAdapter(_sw);
  } else {
    if (_hw == nullptr) {
      _hw = &Serial;
    }
    _serial = new HardwareSerialAdapter(_hw);
  }
}
#elif defined(ESP32)
YFPS2UART::YFPS2UART(uint8_t rxPin, uint8_t txPin, HardwareSerial* hwSerial)
  : _serialType(SERIALTYPE_HW), _rxPin(rxPin), _txPin(txPin),
    _lastReceiveTime(0), _newData(false),
    _ignoreIncoming(false),
    _receiving(false), _ndx(0), _pendingStart(false),
    _rawButtons(0), _stableButtons(0),
    _prevStableButtons(0), _pressedEvents(0), _releasedEvents(0),
    _changedEvents(0), _lastButtons(0),
    _debounceStartMs(0), _debounceMs(30),
    _leftX(128), _leftY(127), _rightX(128), _rightY(127),
    _hw(hwSerial)
{
  _serial = new HardwareSerialAdapter(_hw, _rxPin, _txPin);
}
#endif

YFPS2UART::~YFPS2UART() {
#if defined(__AVR__) 
  if (_serial) {
    delete _serial;
    _serial = nullptr;
  }
#elif defined(ESP32)
  if (_serial) {
    delete _serial;
    _serial = nullptr;
  }
#endif
}


void YFPS2UART::begin(unsigned long espBaud) {
  if (_serial) {
    _serial->begin(espBaud);
  }
}

unsigned int YFPS2UART::getButtons() {
  // 返回去抖后的稳定值
  return (unsigned int)_stableButtons;
}

unsigned int YFPS2UART::getRawButtons() { // 返回未去抖最近一帧的原始值
  return (unsigned int)_rawButtons;
}

void YFPS2UART::setDebounceMs(uint16_t ms) {
  _debounceMs = ms;
}

bool YFPS2UART::hasRecentData(uint32_t timeoutMs) const {
  if (_lastReceiveTime == 0) return false;
  return (millis() - _lastReceiveTime) <= timeoutMs;
}


/*
 * 在 update() 中处理去抖：当收到完整帧（_newData）时解析 rawButtons，
 * 如果与上次 raw 不同则重置去抖计时；当 raw 在 _debounceMs 内保持不变则更新 stableButtons。
 */
void YFPS2UART::update() {
  readDataFromSerial();

  if (_newData) {
    // 根据PS2_ESP32C3_UART.ino的数据包格式解析：
    // 0x0D + buttonsHigh + buttonsLow + leftY + leftX + rightY + rightX + 0x0A
    uint16_t raw = 0;
    raw = ((uint16_t)_buf[0] << 8) | (uint16_t)_buf[1]; // 合并高字节和低字节为16位

    // 解析摇杆（根据你的协议：buf[2]=leftY, buf[3]=leftX, buf[4]=rightY, buf[5]=rightX）
    // 解析摇杆数据（注意_end_MA(0x0A)不应该作为数据使用）
    _leftY = _buf[2];
    _leftX = _buf[3];
    _rightY = _buf[4];
    _rightX = _buf[5];
    // _leftY =127;
    // _leftX = 128 ;
    // _rightY = 127;
    // _rightX = 128;

    // 若 raw 变化，重置去抖计时
    if (raw != _rawButtons) {
      _rawButtons = raw;
      _debounceStartMs = millis();
    } else {
      // 若 raw 保持不变并且已超过去抖时间，则接受该值
      if ((int32_t)(millis() - _debounceStartMs) >= (int32_t)_debounceMs) {
        if (_stableButtons != raw) {
          // 先保存当前的_stableButtons作为前一个状态
          _lastButtons = _stableButtons;
          
          
          // 更新稳定按键值
          _stableButtons = raw;

          // 计算按键状态变化
          uint16_t changed = _lastButtons ^ _stableButtons;
          
          // 处理边沿事件：计算按下 / 释放
          uint16_t pressed = (_stableButtons & ~_lastButtons);
          uint16_t released = (_lastButtons & ~_stableButtons);
          if (pressed) {
            _pressedEvents |= pressed;
            // 记录按住起始时间
            for (int b = 0; b < 16; ++b) {
              if (pressed & (1u << b)) _holdStartMs[b] = millis();
            }
          }
          if (released) {
            _releasedEvents |= released;
            // 清除按住起始时间
            for (int b = 0; b < 16; ++b) {
              if (released & (1u << b)) _holdStartMs[b] = 0;
            }
          }
          // 更新状态变化事件
          if (changed) {
            _changedEvents |= changed;
          }
          
          // 更新前一个稳定状态
          _prevStableButtons = _stableButtons;
        }
      }
    }

    // 处理完成，清标志（注意：去抖可能仍在进行，但 _rawButtons 已更新）
    _newData = false;
    _lastReceiveTime = millis();
  }
}

void YFPS2UART::readDataFromSerial() {
  if (!_serial) return;

  const byte start_MA = 0x0D;
  const byte end_MA = 0x0A;
  int rb;

  // 如果上次检测到 start，但未实际处理，先设置接收状态
  if (_pendingStart) {
    _receiving = true;
    _ndx = 0;
    _pendingStart = false;
  }

  while (_serial->available() > 0 && _newData == false) {
    _lastReceiveTime = millis();
    rb = _serial->read();
    if (rb < 0) break;

    // 如果收到断开标识 0xAB -> 进入忽略模式（不处理后续非协议数据）
    if ((uint8_t)rb == 0xAB) {
      _receiving = false;
      _ndx = 0;
      _newData = false;
      _ignoreIncoming = true;
      // 丢弃并继续
      continue;
    }

    // 忽略模式：直到下一个起始字节 0x0D 出现才恢复处理
    if (_ignoreIncoming) {
      if ((uint8_t)rb == start_MA) {
        // 标记待处理的 start，交由本函数下一轮或后续处理开始接收
        _ignoreIncoming = false;
        _receiving = true;
        _ndx = 0;
      }
      // 否则一直丢弃字节
      continue;
    }

    // 正常接收帧逻辑
    if (_receiving) {
      if ((uint8_t)rb != end_MA) {
        _buf[_ndx] = (byte)rb;
        _ndx++;
        if (_ndx >= sizeof(_buf)) _ndx = sizeof(_buf) - 1;
      } else {
        // 结束符到达，写入并标志新数据
        _buf[_ndx] = end_MA;
        _receiving = false;
        _ndx = 0;
        _newData = true;
      }
    } else if ((uint8_t)rb == start_MA) {
      // 遇到起始字节，进入接收状态
      _receiving = true;
      _ndx = 0;
    }
  }
}

/*
 * 函数: isRemoteConnected
 * 功能: 主动读取串口判断远端是否为 connected（非 0xAB），
 *       并在发现 start_MA 时准备进入接收（设置 _pendingStart）。
 * 参数: 无
 * 返回值:
 *   - true = 远端已连接（非 0xAB 忽略模式），false = 远端断开（正在忽略数据）。
 */
bool YFPS2UART::isRemoteConnected() const {
  if (!_serial) return false;

  YFPS2UART* self = const_cast<YFPS2UART*>(this);

  const uint8_t start_MA = 0x0D;
  const uint8_t disconnect = 0xAB;

  while (_serial->available() > 0) {
    int c = _serial->read();
    if (c < 0) break;
    uint8_t cb = (uint8_t)c;

    if (cb == disconnect) {
      // 收到断开，进入忽略模式
      self->_ignoreIncoming = true;
      self->_receiving = false;
      self->_ndx = 0;
      continue;
    }

    if (cb == start_MA) {
      // 收到起始字节，标记 pendingStart，让 readDataFromSerial 在下一次运行时开始接收
      self->_pendingStart = true;
      self->_ignoreIncoming = false;
      break; // 发现 start 后可以退出，后续帧由 readDataFromSerial 处理
    }

    // 其它字节在此处直接丢弃
  }

  return !self->_ignoreIncoming;
}


/**
 * @brief 发送震动控制命令到UART设备
 * 
 * 该函数通过UART接口向连接的设备发送震动控制命令。
 * 只有在串口对象有效时才会执行发送操作。
 * 
 * @param cmd 震动控制命令字节，单字节命令值（0x01 双马达、0x02 左马达、0x03 右马达）
 */
void YFPS2UART::sendVibrate(uint8_t cmd) {
#if defined(__AVR__) 
  if (_serial) {
    if (_serialType == SERIALTYPE_HW) {
      while (_serial->available() > 0) {
        _serial->read();
      }
    }
    _serial->write(cmd);
    if (_serialType == SERIALTYPE_HW) {
      _serial->flush();
    }
  }
#elif defined(ESP32)
  if (_serial) {
    _serial->write(cmd);
  }
#endif
}



// 检查是否有任何按键状态改变
// bool YFPS2UART::NewButtonState() {
//   // // 使用_changedEvents作为独立的事件源
//   // bool hasChanges = _changedEvents != 0;
//   // if (hasChanges) {
//   //   _changedEvents = 0;  // 清除所有变化事件标志
//   // }
//   // return hasChanges;

//   // 与PS2X库保持一致，直接使用异或操作检测变化
//   return ((_lastButtons ^ _stableButtons) > 0);
// }

// 检查特定按键是否有状态改变
// bool YFPS2UART::NewButtonState(uint16_t button) {
//   // // 使用_changedEvents作为独立的事件源
//   // uint16_t hit = _changedEvents & button;
//   // if (hit) {
//   //   _changedEvents &= ~hit;  // 清除对应按键的变化事件标志
//   //   return true;
//   // }
//   // return false;
  
//   // 与PS2X库保持一致，直接使用异或操作检测特定按键变化
//   return (((_lastButtons ^ _stableButtons) & button) > 0);
// }

// 检查按键当前是否被按下
bool YFPS2UART::Button(uint16_t button) {
  return ((_stableButtons & button) > 0);
}

// 检查按键是否刚被按下
bool YFPS2UART::ButtonPressed(uint16_t button) {
  // 使用_pressedEvents来检测刚按下的按键，并在读取后清除事件
  uint16_t hit = _pressedEvents & button;
  if (hit) {
    _pressedEvents &= ~hit;  // 读取后清除对应事件位
    return true;
  }
  return false;

  // 与PS2X库保持一致，使用NewButtonState和Button的组合
  // return(NewButtonState(button) & Button(button));
}

// bool YFPS2UART::wasPressed(uint16_t mask) {
//   uint16_t hit = _pressedEvents & mask;
//   if (hit) _pressedEvents &= ~hit; // 读取后清除对应事件位
//   return (hit != 0);
// }

// 检查按键是否刚被释放
bool YFPS2UART::ButtonReleased(uint16_t button) {
  // 使用_releasedEvents来检测刚释放的按键，并在读取后清除事件
  uint16_t hit = _releasedEvents & button;
  if (hit) {
    _releasedEvents &= ~hit;  // 读取后清除对应事件位
    return true;
  }
  return false;

  //   // 与PS2X库保持一致，使用NewButtonState和(~_prevStableButtons)的组合
  // return((NewButtonState(button)) & ((~_lastButtons & button) > 0));
}

// bool YFPS2UART::wasReleased(uint16_t mask) {
//   uint16_t hit = _releasedEvents & mask;
//   if (hit) _releasedEvents &= ~hit;
//   return (hit != 0);
// }

/* 
 * 获取摇杆值（0-255）
 * @param axis 摇杆轴，使用PSS_LX/PSS_LY/PSS_RX/PSS_RY常量
 * @return 摇杆值（0-255）
*/
uint8_t YFPS2UART::Analog(byte axis) {
  // 与PS2X库保持一致的返回值范围（0-255）
  if (axis == PSS_LX) return _leftX;
  if (axis == PSS_LY) return _leftY;
  if (axis == PSS_RX) return _rightX;
  if (axis == PSS_RY) return _rightY;
  return 0;
}



/*
 * 函数: sendATCommand
 * 功能: 将指定的 ASCII 命令发送到对端（通过 SoftwareSerial），并追加 CR+LF。
 * 参数:
 *   - cmd (const char*) : 要发送的 AT 命令（例如 "AT+RST" 或 "AT+BAUD=115200"）。
 * 返回值:
 *   - 无（void）。若未初始化 _sw 则直接返回。
 */
void YFPS2UART::sendATCommand(const char *cmd) {
  if (!_serial) return;
  
  while (_serial->available() > 0) {
    _serial->read();
  }
  
  _serial->print(cmd);
  _serial->print("\r\n");
  
  _serial->flush();
}

/*
 * 函数: sendResetCommand
 * 功能: 发送软件复位指令 AT+RST 到对端。
 * 参数:
 *   - 无
 * 返回值:
 *   - 无（void）。
 */
void YFPS2UART::sendResetCommand() {
  sendATCommand("AT+RST");
}

/*
 * 函数: sendSetBaud
 * 功能: 向对端发送 AT+BAUD=<baud> 指令以请求对端切换波特率。
 * 参数:
 *   - baud (uint32_t) : 要设置的波特率（库中仅允许 9600 或 115200）。
 * 返回值:
 *   - bool : true 表示命令已发送（参数合法），false 表示不支持该波特率。
 * 说明:
 *   - 实际对端是否切换取决于对端固件；若对端切换且 reinitLocal=true，则本地也会切换以继续通信。
 */
bool YFPS2UART::sendSetBaud(uint32_t baud) {
  if (baud != 9600 && baud != 115200) {
    return false;
  }

  char cmdBuf[32];
  snprintf(cmdBuf, sizeof(cmdBuf), "AT+BAUD=%lu", (unsigned long)baud);
  sendATCommand(cmdBuf);

  return true;
}


/*
 * 函数: sendATCommandWithResponse
 * 功能: 发送 ASCII AT 命令并在指定超时内读取对端返回的一行（以 '\\n' 结束）。
 * 参数:
 *   - cmd (const char*): 要发送的 AT 命令，不包含 CR/LF（例如 "AT+VER"）
 *   - respBuf (char*): 接收缓冲区，函数会写入 NUL 结尾字符串
 *   - bufLen (size_t): 缓冲区长度（至少 2）
 *   - timeoutMs (uint32_t): 等待响应的超时时间（毫秒）
 * 返回值:
 *   - bool: true 表示收到响应并写入 respBuf（去掉末尾 CR/LF），false 表示超时或错误。
 */
bool YFPS2UART::sendATCommandWithResponse(const char *cmd, char *respBuf, size_t bufLen, uint32_t timeoutMs) {
  if (!_serial || !respBuf || bufLen < 2) return false;

  while (_serial->available() > 0) {
    _serial->read();
  }

  _serial->print(cmd);
  _serial->print("\r\n");
  
  _serial->flush();

  size_t idx = 0;
  unsigned long start = millis();
  bool gotLine = false;
  while (millis() - start < timeoutMs) {
    while (_serial->available() > 0) {
      int c = _serial->read();
      if (c < 0) continue;

      if ((uint8_t)c == 0xAB) {
        continue;
      }

      if (idx < bufLen - 1) {
        respBuf[idx++] = (char)c;
      }
      if (c == '\n') {
        gotLine = true;
        break;
      }
    }
    if (gotLine) break;
  }

  if (idx >= bufLen) idx = bufLen - 1;
  respBuf[idx] = '\0';

  if (!gotLine) {
    return false;
  }

  while (idx > 0 && (respBuf[idx - 1] == '\n' || respBuf[idx - 1] == '\r')) {
    respBuf[--idx] = '\0';
  }

  return true;
}

/*
 * 函数: queryBaudRate
 * 功能: 查询模块当前的波特率设置
 * 参数:
 *   - baudRate (uint32_t&) : 用于返回查询到的波特率值
 *   - timeoutMs (uint32_t): 等待响应的超时时间（毫秒）
 * 返回值:
 *   - bool: true 表示成功查询到波特率，false 表示查询失败或超时
 * 说明:
 *   - 发送 AT+BAUD? 命令并解析返回的波特率值
 */
bool YFPS2UART::queryBaudRate(uint32_t& baudRate, uint32_t timeoutMs) {
  char respBuf[32];
  
  bool ok = sendATCommandWithResponse("AT+BAUD?", respBuf, sizeof(respBuf), timeoutMs);
  
  if (!ok) {
    return false;
  }
  
  baudRate = (uint32_t)atol(respBuf);
  
  if (baudRate == 0) {
    return false;
  }
  
  return true;
}

/*
 * 函数: sendATCommandPrintResponse
 * 功能: 发送 AT 指令并把对端响应打印到主串口 Serial（使用内部临时缓冲）。
 * 参数:
 *   - cmd (const char*): 要发送的 AT 指令
 *   - timeoutMs (uint32_t): 超时时间（毫秒）
 * 返回值:
 *   - bool: true 表示收到并打印响应，false 表示未收到。
 */
bool YFPS2UART::sendATCommandPrintResponse(const char *cmd, uint32_t timeoutMs) {
  char buf[128];
  return sendATCommandWithResponse(cmd, buf, sizeof(buf), timeoutMs);
}


// void YFPS2UART::readData() {
//   static boolean ZZS = false;
//   static byte ndx = 0;
//   byte start_MA = 0x0D;
//   byte end_MA = 0x0A;
//   byte rb;

//   while (mySerial.available() > 0 && newData == false) {
//     lastReceiveTime = millis();
//     rb = mySerial.read();
//     if (ZZS == true) {
//       if (rb != end_MA) {
//         Read_Bytes[ndx] = rb;
//         ndx++;
//         if (ndx >= 8) {
//           ndx = 7;
//         }
//       } else {
//         Read_Bytes[ndx] = end_MA;
//         ZZS = false;
//         ndx = 0;
//         newData = true;
//       }
//     } else if (rb == start_MA) {
//       ZZS = true;
//     }
//   }
// }
