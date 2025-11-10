/*
 * YFPS2UART_Demo.ino
 * 演示 YFPS2UARTLib 库的基本用法，包括发送复位指令、固件版本查询、
 * 获取按键、摇杆信息、打印所有按键的按下和释放状态、发送 AT 命令；
 * 新增：按下X键、O键和方形键时触发不同震动模式的功能
 * 
 * @ YFROBOT
 * @ 2025-11-06
*/
#include <YFPS2UART.h>

// Arduino UNO R3 引脚配置
YFPS2UART ps2uart(11, 10);  // RX, TX (根据硬件调整)

// 手柄连接成功，只输出一次提示
static bool connectedNotified = false;

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println(F("YFPS2UART 完整按键演示"));
  Serial.println(F("====================================="));
  Serial.println(F("本示例演示所有PS2按键的按下和释放状态"));
  Serial.println(F("按下L1或R1时将打印摇杆值"));
  Serial.println(F("====================================="));

  ps2uart.setDebug(false);
  ps2uart.setDebounceMs(10);  // 可调整去抖时间，默认 10ms
  ps2uart.begin(115200);      // 9600 或 115200，必须与模块波特保持一致，否则无法通讯
  delay(100);

  // // 软件复位演示（功能测试，可选取消）
  // Serial.println("指令复位模块，测试指令可取消...");
  // ps2uart.sendResetCommand();
  // delay(2000);  // 必要等待模块重启完成

  // 查询模块固件版本（功能测试，可选取消）
  char verBuf[32]; // 减小缓冲区大小
  Serial.println(F("获取模块固件版本，测试指令可取消..."));
  if (ps2uart.sendATCommandWithResponse("AT+VER", verBuf, sizeof(verBuf), 500)) {
    delay(500);
    Serial.print(F("PS2 UART VER: "));
    Serial.println(verBuf);
  } else {
    Serial.println(F("无法获取固件版本，请检查连接！"));
  }

  Serial.println();
  Serial.println(F("初始化完成，请开始操作手柄..."));
  Serial.println();
}

void loop() {

  // 若手柄未连接状态，定期提示
  if (!ps2uart.isRemoteConnected()) {
    static unsigned long lastTime = 0;
    if (millis() - lastTime >= 2000) {
      lastTime = millis();
      connectedNotified = false;
      Serial.print(F("等待手柄连接..."));
      Serial.println(lastTime);
    }
    return;
  } else {
    if (!connectedNotified) {
      connectedNotified = true;
      Serial.println(F("手柄连接成功！"));
      delay(10);
    }
  }

  // 更新读取手柄数据
  ps2uart.update();

  // 获取按键原始值(去抖)
  // unsigned int buttons = ps2uart.getButtons();

  // 演示所有按键的按下和释放状态 - 每个按键都单独显示按下和释放状态
  if (ps2uart.Button(PSB_START))  //will be TRUE as long as button is pressed
    Serial.println(F("Start is being held"));
  if (ps2uart.Button(PSB_SELECT))
    Serial.println(F("Select is being held"));

  if (ps2uart.Button(PSB_PAD_UP)) {  //will be TRUE as long as button is pressed
    Serial.println(F("Up is being held"));
  }
  if (ps2uart.Button(PSB_PAD_RIGHT)) {
    Serial.println(F("Right is being held"));
  }
  if (ps2uart.Button(PSB_PAD_LEFT)) {
    Serial.println(F("Left is being held"));
  }
  if (ps2uart.Button(PSB_PAD_DOWN)) {
    Serial.println(F("Down is being held"));
  }

  if (ps2uart.ButtonPressed(PSB_L3))  //will be TRUE if button was JUST pressed
    Serial.println(F("L3 pressed"));
  if (ps2uart.ButtonPressed(PSB_R3))
    Serial.println(F("R3 pressed"));
  if (ps2uart.ButtonPressed(PSB_L2))
    Serial.println(F("L2 pressed"));
  if (ps2uart.ButtonPressed(PSB_R2))
    Serial.println(F("R2 pressed"));

  if (ps2uart.ButtonReleased(PSB_TRIANGLE))  //will be TRUE if button was JUST released
    Serial.println(F("Triangle just released"));

  // 检测X键 (绿色) - 同时震动两个电机
  if (ps2uart.ButtonPressed(PSB_CROSS)) {
    ps2uart.sendVibrate(VIBRATE_BOTH);
    Serial.println(F("X just pressed"));
      Serial.println(F("X键 (绿色) - 双电机震动"));
  }
  // 检测O键 (红色) - 左电机震动
  if (ps2uart.ButtonPressed(PSB_CIRCLE)) {
    ps2uart.sendVibrate(VIBRATE_LEFT);
    Serial.println(F("Circle just pressed"));
      Serial.println(F("O键 (红色) - 左电机震动"));
  }
  // 检测方形键 (蓝色) - 右电机震动
  if (ps2uart.ButtonPressed(PSB_SQUARE)) {
    ps2uart.sendVibrate(VIBRATE_RIGHT);
    Serial.println(F("Square just pressed"));
      Serial.println(F("方形键 (蓝色) - 右电机震动"));
  }

  // 按住 L1 或 R1 时持续打印摇杆值
  if (ps2uart.Button(PSB_L1) || ps2uart.Button(PSB_R1)) {  //print stick values if either is TRUE
    Serial.print(F("Stick Values:"));
    Serial.print(ps2uart.Analog(PSS_LY), DEC);  //Left stick, Y axis. Other options: LX, RY, RX
    Serial.print(",");
    Serial.print(ps2uart.Analog(PSS_LX), DEC);
    Serial.print(",");
    Serial.print(ps2uart.Analog(PSS_RY), DEC);
    Serial.print(",");
    Serial.println(ps2uart.Analog(PSS_RX), DEC);
  }
  delay(20);
}
