# YFPS2UART 库 [![English Version](https://img.shields.io/badge/-English-blue)](README.en.md)

## 语言切换 (Language Switch)
- [中文 (Chinese)](README.md)
- [英文 (English)](README.en.md)

## 概述
YFPS2UART 是一个功能强大的 Arduino 库，用于通过 UART 串行通信连接 PS2 手柄。该库支持多种 Arduino 兼容板，包括 Arduino UNO 和 ESP32 系列，提供丰富的 API 用于读取按钮状态、摇杆值、发送震动命令等功能。

## 产品购买链接
- 官方网站: [https://www.yfrobot.com/](https://www.yfrobot.com/)
- 淘宝店铺: [https://yfrobot.taobao.com/](https://yfrobot.taobao.com/)

## 特性
- 支持 Arduino UNO 和 ESP32 系列主板
- 自动检测手柄连接状态
- 按键状态读取（按下/释放/按住）
- 摇杆模拟值读取（X/Y 轴）
- 支持双震动电机控制
- 内置按键去抖功能
- 支持 AT 命令通信（查询版本、修改波特率等）
- 内存优化设计，适用于资源有限的平台

## 安装
### 方法一：通过 Arduino IDE 库管理器安装
1. 打开 Arduino IDE
2. 点击 "工具" > "管理库..."
3. 在搜索框中输入 "YFPS2UART"
4. 点击 "安装" 按钮

### 方法二：手动安装
1. 下载本库的 ZIP 文件
2. 解压到 Arduino 库文件夹（通常位于 `Documents/Arduino/libraries`）
3. 重启 Arduino IDE 以识别新库

## 硬件连接

### 接收器和手柄指示灯说明
- 接收器和手柄都有红、绿两颗灯
- 红灯：电源指示灯（通电时常亮）
- 绿灯：连接状态指示灯
  - 闪烁：未连接或正在配对中
  - 常亮：配对连接成功

### 响应时间和时序信息
- **震动持续时间**：每个震动命令持续 0.5 秒
- **手柄配对窗口**：约 2 秒
- **初始化时间**：接收器模块在通电后 1-2 秒内完成初始化
- **手柄休眠时间**：
  - 未连接状态：无操作 15 秒后自动休眠
  - 已连接状态：无操作 2 分钟后自动休眠
- **手柄配对行为**：
  - 不关机：记忆最后配对的接收器，使用 START 键唤醒时自动找寻该接收器，不会连接到其他接收器
  - 关机：不记忆接收器，开机时随机配对连接可用接收器

### Arduino UNO R3 连接方式

### Arduino UNO R3 连接
- 手柄模块 RX 连接到 Arduino TX (10)
- 手柄模块 TX 连接到 Arduino RX (11)
- 手柄模块 VCC 连接到 Arduino 5V
- 手柄模块 GND 连接到 Arduino GND

### ESP32 连接
- 手柄模块 RX 连接到 ESP32 TX (17)
- 手柄模块 TX 连接到 ESP32 RX (16)
- 手柄模块 VCC 连接到 ESP32 5V
- 手柄模块 GND 连接到 ESP32 GND

## 基本用法

### Arduino UNO 示例
```cpp
#include <YFPS2UART.h>

// Arduino UNO R3 引脚配置
YFPS2UART ps2uart(11, 10);  // RX, TX

void setup() {
  Serial.begin(115200);
  ps2uart.setDebug(false);
  ps2uart.setDebounceMs(10);  // 设置去抖时间
  ps2uart.begin(9600);      // 初始化串口通信 UNO软串口请使用9600波特率
  
  Serial.println("YFPS2UART 示例程序已启动");
}

void loop() {
  // 更新手柄数据
  ps2uart.update();
  
  // 检查手柄连接状态
  if (!ps2uart.isRemoteConnected()) {
    Serial.println(F("等待手柄连接..."));
    delay(1000);
    return;
  }
  
  // 读取按键状态
  if (ps2uart.Button(PSB_START)) {
    Serial.println(F("Start 键被按下"));
  }
  
  // 读取摇杆值
  Serial.print(F("摇杆值: LY="));
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

### ESP32 示例
```cpp
#include <YFPS2UART.h>

// ESP32 引脚配置
YFPS2UART ps2uart(16, 17);  // RX, TX

void setup() {
  Serial.begin(115200);
  ps2uart.setDebug(false);
  ps2uart.setDebounceMs(10);  // 设置去抖时间
  ps2uart.begin(9600);      // 初始化串口通信
  
  Serial.println("YFPS2UART ESP32 示例程序已启动");
}

void loop() {
  // 与 Arduino UNO 示例相同
}
```

## API 参考

### 构造函数
- `YFPS2UART(uint8_t rxPin = 11, uint8_t txPin = 10)`: 创建 YFPS2UART 实例，指定 RX 和 TX 引脚

### 初始化和配置
- `void begin(unsigned long espBaud = 9600)`: 初始化库并设置串口通信，默认波特率 9600
- `void setDebug(bool enable)`: 启用/禁用调试信息输出
- `void setDebounceMs(uint16_t ms)`: 设置按键去抖时间（毫秒）

### 数据更新和连接状态
- `void update()`: 更新手柄数据，应在 loop() 中定期调用
- `bool isRemoteConnected() const`: 检查手柄是否已连接
- `bool hasRecentData(uint32_t timeoutMs = 1000) const`: 检查是否有最近的数据更新

### 按键状态查询
- `unsigned int getButtons()`: 返回去抖后的稳定按键值
- `unsigned int getRawButtons()`: 返回未去抖的原始按键值
- `bool Button(uint16_t button)`: 检查指定按键是否被按住
- `bool ButtonPressed(uint16_t button)`: 检查指定按键是否刚被按下（边缘检测）
- `bool ButtonReleased(uint16_t button)`: 检查指定按键是否刚被释放（边缘检测）

### 摇杆值读取
- `uint8_t Analog(byte axis)`: 返回指定摇杆轴的模拟值（0-255）
  - PSS_LY: 左摇杆 Y 轴
  - PSS_LX: 左摇杆 X 轴
  - PSS_RY: 右摇杆 Y 轴
  - PSS_RX: 右摇杆 X 轴

### 震动控制
- `void sendVibrate(uint8_t cmd)`: 发送震动命令
  - VIBRATE_BOTH: 双电机震动
  - VIBRATE_LEFT: 左电机震动
  - VIBRATE_RIGHT: 右电机震动

### AT 命令
- `void sendATCommand(const char *cmd)`: 发送 AT 命令
- `void sendResetCommand()`: 发送软件复位命令
- `bool sendSetBaud(uint32_t baud)`: 设置波特率（支持 9600, 19200, 38400, 57600, 115200）
- `bool sendATCommandWithResponse(const char *cmd, char *respBuf, size_t bufLen, uint32_t timeoutMs = 500)`: 发送命令并读取响应
- `bool queryBaudRate(uint32_t& baudRate, uint32_t timeoutMs = 500)`: 查询当前波特率

## 按键定义
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

## 示例程序
库包含以下示例程序：
- `YFPS2UART_Demo`: 基本功能演示，包括按键和摇杆读取
- `YFPS2UART_Demo_ChangeBAUD`: 波特率修改示例
- `YFPS2UART_ESP_Demo`: ESP32 平台专用示例
- `YFPS2UART_ESP_Demo_ChangeBAUD`: ESP32 平台波特率修改示例

## 内存优化
本库针对资源有限的平台（如 Arduino UNO）进行了优化：
- 使用 F() 宏存储字符串到 Flash 而非 RAM
- 避免使用 String 类，减少动态内存分配
- 最小化缓冲区大小
- 移除未使用的成员变量

## 故障排除
1. **连接问题**：确保 RX/TX 引脚连接正确，波特率匹配
2. **数据不更新**：检查手柄是否已配对，模块电源是否正常
3. **内存不足**：使用 F() 宏存储字符串，减少不必要的变量
4. **按键抖动**：调整 setDebounceMs() 参数

## 更新日志
- 1.0.1: 修复 UNO 软串口波特率问题，默认波特率改为 9600
- 1.0.0: 初始版本

## 许可
本库采用 MIT 许可协议。详情请参阅 extras 目录中的 LICENSE 文件。

## 支持
如有任何问题或建议，请在 GitHub 仓库中提交 issue 或 pull request。