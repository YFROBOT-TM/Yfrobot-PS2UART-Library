#include "YFPS2UART_HW.h"

YFPS2UART_HW::YFPS2UART_HW()
  : _serial(&Serial),
    _lastReceiveTime(0),
    _newData(false),
    _ignoreIncoming(false),
    _receiving(false),
    _ndx(0),
    _pendingStart(false),
    _rawButtons(0),
    _stableButtons(0),
    _debounceStartMs(0),
    _debounceMs(30),
    _lastButtons(0),
    _prevStableButtons(0),
    _pressedEvents(0),
    _releasedEvents(0),
    _changedEvents(0),
    _leftX(128),
    _leftY(127),
    _rightX(128),
    _rightY(127)
{
  memset(_holdStartMs, 0, sizeof(_holdStartMs));
}

void YFPS2UART_HW::begin(unsigned long baud) {
  if (_serial) {
    _serial->begin(baud);
    delay(100);
    
    while (_serial->available() > 0) {
      _serial->read();
    }
  }
}

void YFPS2UART_HW::setDebounceMs(uint16_t debounceMs) {
  _debounceMs = debounceMs;
}

uint16_t YFPS2UART_HW::getDebounceMs() const {
  return _debounceMs;
}

bool YFPS2UART_HW::hasRecentData(uint32_t timeoutMs) const {
  if (_lastReceiveTime == 0) return false;
  return (millis() - _lastReceiveTime) <= timeoutMs;
}

void YFPS2UART_HW::update() {
  readDataFromSerial();

  if (_newData) {
    uint16_t raw = 0;
    raw = ((uint16_t)_buf[0] << 8) | (uint16_t)_buf[1];

    _leftY = _buf[2];
    _leftX = _buf[3];
    _rightY = _buf[4];
    _rightX = _buf[5];

    if (raw != _rawButtons) {
      _rawButtons = raw;
      _debounceStartMs = millis();
    } else {
      if ((int32_t)(millis() - _debounceStartMs) >= (int32_t)_debounceMs) {
        if (_stableButtons != raw) {
          _lastButtons = _stableButtons;
          _stableButtons = raw;

          uint16_t changed = _lastButtons ^ _stableButtons;
          uint16_t pressed = (_stableButtons & ~_lastButtons);
          uint16_t released = (_lastButtons & ~_stableButtons);
          
          if (pressed) {
            _pressedEvents |= pressed;
            for (int b = 0; b < 16; ++b) {
              if (pressed & (1u << b)) _holdStartMs[b] = millis();
            }
          }
          
          if (released) {
            _releasedEvents |= released;
            for (int b = 0; b < 16; ++b) {
              if (released & (1u << b)) _holdStartMs[b] = 0;
            }
          }
          
          if (changed) {
            _changedEvents |= changed;
          }
          
          _prevStableButtons = _stableButtons;
        }
      }
    }

    _newData = false;
    _lastReceiveTime = millis();
  }
}

void YFPS2UART_HW::readDataFromSerial() {
  if (!_serial) return;

  const byte start_MA = 0x0D;
  const byte end_MA = 0x0A;
  int rb;

  if (_pendingStart) {
    _receiving = true;
    _ndx = 0;
    _pendingStart = false;
  }

  while (_serial->available() > 0 && _newData == false) {
    _lastReceiveTime = millis();
    rb = _serial->read();
    if (rb < 0) break;

    if ((uint8_t)rb == 0xAB) {
      _receiving = false;
      _ndx = 0;
      _newData = false;
      _ignoreIncoming = true;
      continue;
    }

    if (_ignoreIncoming) {
      if ((uint8_t)rb == start_MA) {
        _ignoreIncoming = false;
        _receiving = true;
        _ndx = 0;
      }
      continue;
    }

    if (_receiving) {
      if ((uint8_t)rb != end_MA) {
        _buf[_ndx] = (byte)rb;
        _ndx++;
        if (_ndx >= sizeof(_buf)) _ndx = sizeof(_buf) - 1;
      } else {
        _buf[_ndx] = end_MA;
        _receiving = false;
        _ndx = 0;
        _newData = true;
      }
    } else if ((uint8_t)rb == start_MA) {
      _receiving = true;
      _ndx = 0;
    }
  }
}

bool YFPS2UART_HW::isRemoteConnected() const {
  if (!_serial) return false;

  YFPS2UART_HW* self = const_cast<YFPS2UART_HW*>(this);

  const uint8_t start_MA = 0x0D;
  const uint8_t disconnect = 0xAB;

  while (_serial->available() > 0) {
    int c = _serial->read();
    if (c < 0) break;
    uint8_t cb = (uint8_t)c;

    if (cb == disconnect) {
      self->_ignoreIncoming = true;
      self->_receiving = false;
      self->_ndx = 0;
      continue;
    }

    if (cb == start_MA) {
      self->_pendingStart = true;
      self->_ignoreIncoming = false;
      break;
    }
  }

  return !self->_ignoreIncoming;
}

void YFPS2UART_HW::sendVibrate(uint8_t cmd) {
  if (_serial) {
    while (_serial->available() > 0) {
      _serial->read();
    }
    _serial->write(cmd);
    _serial->flush();
  }
}

bool YFPS2UART_HW::Button(uint16_t button) {
  return ((_stableButtons & button) > 0);
}

bool YFPS2UART_HW::ButtonPressed(uint16_t button) {
  uint16_t hit = _pressedEvents & button;
  if (hit) {
    _pressedEvents &= ~hit;
    return true;
  }
  return false;
}

bool YFPS2UART_HW::ButtonReleased(uint16_t button) {
  uint16_t hit = _releasedEvents & button;
  if (hit) {
    _releasedEvents &= ~hit;
    return true;
  }
  return false;
}

bool YFPS2UART_HW::NewButtonState() {
  return ((_lastButtons ^ _stableButtons) > 0);
}

bool YFPS2UART_HW::ButtonChanged(uint16_t button) {
  return (((_lastButtons ^ _stableButtons) & button) > 0);
}

uint8_t YFPS2UART_HW::Analog(byte axis) {
  if (axis == PSS_LX) return _leftX;
  if (axis == PSS_LY) return _leftY;
  if (axis == PSS_RX) return _rightX;
  if (axis == PSS_RY) return _rightY;
  return 0;
}
