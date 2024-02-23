//#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <ESP32Servo.h>
Servo servo;
const int STBY = 17; // モータードライバの制御の準備
const int AIN1 = 15; // 1つ目のDCモーターの制御
const int AIN2 = 4; // 1つ目のDCモーターの制御
const int BIN1 = 5; // 2つ目のDCモーターの制御
const int BIN2 = 18; // 2つ目のDCモーターの制御
const int PWMA = 0; // 1つ目のDCモーターの回転速度
const int PWMB = 19; // 2つ目のDCモーターの回転速度
const int  SERVO_PIN = 13;
void setup() {
pinMode(AIN1, OUTPUT);
pinMode(AIN2, OUTPUT);
pinMode(BIN1, OUTPUT);
pinMode(BIN2, OUTPUT);
digitalWrite(STBY, HIGH);
// スタンバイ
pinMode(STBY, OUTPUT);
pinMode(PWMA, OUTPUT);
pinMode(PWMB, OUTPUT);
// put your setup code here, to run once:
servo.attach(SERVO_PIN,510,2400); //サーボモーターの初期化
}
void loop() {
//アーム展開

servo.write(90);
delay(500);
//servo.attach(26);
//servo.write(170);
//servo.attach(27);
//servo.write(30);
// 回転速度を設定（0～255）まで
analogWrite(PWMA, 150);
analogWrite(PWMB, 150);
// 前進（後進は全て逆にする）
digitalWrite(AIN1, LOW);
digitalWrite(AIN2, HIGH);
digitalWrite(BIN1, HIGH);
digitalWrite(BIN2, LOW);
delay(5000);
//停止
digitalWrite(AIN1, LOW);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, LOW);
//後進
digitalWrite(AIN1, HIGH);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, HIGH);
delay(500);
//停止
digitalWrite(AIN1, LOW);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, LOW);
//回収
/*
servo.write(70);
delay(500);
servo.attach(26);
servo.write(120);
delay(500);
servo.attach(25);
servo.write(90);
delay(500);
*/
// 右回り（左回りは逆）
digitalWrite(AIN1, HIGH);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, HIGH);
digitalWrite(BIN2, LOW);
delay(3000);
//停止
digitalWrite(AIN1, LOW);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, LOW);
//設置
/*
servo.write(180);
delay(500);
servo.attach(26);
servo.write(170);
servo.attach(27);
servo.write(30);
delay(500);
*/
//後進
digitalWrite(AIN1, HIGH);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, HIGH);
delay(1500);
//停止
digitalWrite(AIN1, LOW);
digitalWrite(AIN2, LOW);
digitalWrite(BIN1, LOW);
digitalWrite(BIN2, LOW);
/*
//アーム収納
servo.attach(27);
servo.write(70);
delay(500);
servo.attach(26);
servo.write(120);
delay(500);
servo.attach(25);
servo.write(90);
delay(500);
*/
//
delay(9999999);
// put your main code here, to run repeatedly:
}