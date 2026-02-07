/*
  YFPS2UARTHWLib_Demo
  使用 UNO 硬件串口进行串口通讯，解决软串口库与舵机库发生冲突问题
  本示例演示如何使用 YFPS2UART_HW 库来控制PS2无线模块，
  并通过串口输出PS2模块的按键状态。

  YFPS2UART库也兼容硬件串口功能，留存参考。
  @yfrobot 2026-02-06
*/
#include <YFPS2UART_HW.h>
#include <Servo.h>

YFPS2UART_HW ps2uart;
Servo myServo;
#define LED_PIN 13

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  ps2uart.setDebounceMs(10);
  ps2uart.begin(9600);
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    delay(50);
  }
  
  myServo.attach(3);
  myServo.write(90);
}

void loop() {
  ps2uart.update();
  
  int stickX = ps2uart.Analog(PSS_LX);
  int angle = map(stickX, 0, 255, 0, 180);
  myServo.write(angle);
  
  if (ps2uart.ButtonPressed(PSB_CROSS)) {
    ps2uart.sendVibrate(VIBRATE_BOTH);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
  }
  
  if (ps2uart.ButtonPressed(PSB_CIRCLE)) {
    ps2uart.sendVibrate(VIBRATE_LEFT);
  }
  
  if (ps2uart.ButtonPressed(PSB_TRIANGLE)) {
    ps2uart.sendVibrate(VIBRATE_RIGHT);
  }
  
  if (ps2uart.ButtonPressed(PSB_SQUARE)) {
    ps2uart.sendVibrate(VIBRATE_BOTH);
  }
  
  delay(40);
}
