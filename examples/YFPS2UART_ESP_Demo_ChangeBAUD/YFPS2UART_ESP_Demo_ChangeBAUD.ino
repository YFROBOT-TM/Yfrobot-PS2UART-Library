/*
 * YFPS2UART_Demo_ChangeBAUD.ino
 * 演示 YFPS2UARTLib 库的基本用法，修改波特率，支持9600, 19200, 38400, 57600, 115200波特率
 * 
 * 波特率修改后，切记以下几点，以保证通信正常（重要）
 *    1、如果波特率修改成功，当前程序就已经无法通讯了。
 *    2、需要将模块断电重启
 *    3、需要将程序中的波特率重新配置为修改后的值，重新上传程序
 * 
 * @ YFROBOT
 * @ 2025-11-06
*/

#include <YFPS2UART.h>

// ESP32 引脚配置
YFPS2UART ps2uart(16, 17);  // RX, TX (根据硬件调整)

// 当前波特率，默认9600，支持波特例: 9600, 19200, 38400, 57600, 115200
uint32_t currentBaud = 9600;
uint32_t TARGET_BAUD = (currentBaud == 9600) ? 115200 : 9600;  // 目标波特率，与当前波特率相反

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("修改波特率例程");  // 支持波特例: 9600, 19200, 38400, 57600, 115200

  // ps2uart.setDebug(true);  // 调试信息打印。

  ps2uart.begin(currentBaud);  // 必须与模块波特保持一致，否则无法通讯

  if (ps2uart.queryBaudRate(currentBaud)) {  //查询模块当前波特率
    Serial.println("查询到当前模块波特率: " + String(currentBaud));
    delay(1000);

    // 默认被注释，需要改波特率，取消下面程序注释即可。

    // /**********************修改波特率 ***************/
    // // 发送 AT+BAUD=TARGET_BAUD ，将模块波特率修改为TARGET_BAUD
    // bool ok = ps2uart.sendSetBaud(TARGET_BAUD);  // 仅允许 9600 或 115200
    // Serial.println("修改模块波特率至：" + String(TARGET_BAUD));
    // Serial.println("请将模块断电重启！！！！！");
    // if (!ok) {
    //   Serial.println("注意：sendSetBaud 仅支持 9600 与 115200，或命令未成功发送。");
    // }
    // /**********************修改波特率 ***************/

  } else {
    Serial.println("查询波特率失败！检查波特率配置是否正确，模块是否断电重启！");
  }
}

void loop() {
  // nothing
  delay(1000);
}