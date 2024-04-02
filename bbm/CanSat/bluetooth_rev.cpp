#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
//bluetooth ready
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

String pre_rev_data[2]; //受信データの一時保存用
int rev_data[2] = {0,0};

typedef struct{
    int in1;
    int in2;
    int pwm;
} MotorPin;



void MotorOut(MotorPin pin[], int speed, int omega){
    //モーターの速度調整
    int speed_L,speed_R;
    if(speed + omega > 255 || speed - omega < 255){
        speed = speed * (255 - omega) / abs(speed);
    }
    speed_R = speed + omega;
    speed_L = speed - omega;

    int MotorSpeed[2] = {speed_L,speed_R};

    //出力
    for(int i = 0; i < 2; i++){
        if(MotorSpeed[i] > 0){
            digitalWrite(pin[i].in1, HIGH);
            digitalWrite(pin[i].in2, LOW);
            analogWrite(pin[i].pwm, MotorSpeed[i]);
        }else if(MotorSpeed[i] < 0){
            digitalWrite(pin[i].in1, LOW);
            digitalWrite(pin[i].in2, HIGH);
            analogWrite(pin[i].pwm, -MotorSpeed[i]);
        }else{
            digitalWrite(pin[i].in1, LOW);
            digitalWrite(pin[i].in2, LOW);
            analogWrite(pin[i].pwm, 0);
        }
    }
}
void ziritu(){
    //自律制御
    //ここに自律制御のプログラムを書く
    MoterOut(MOTOR_PIN, 255, 0);
}
void receiveData(String data){
    int index = 0; 
    int datalength = data.length();
    //文字列データの初期化
    for (int i = 0; i < data.length(); i++) {
        char tmp = data.charAt(i);
        if (tmp == ',') {
            index++;
        } else {
            pre_rev_data[index] += tmp;
        }
    }
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin("sinkan");
    Serial.println("device start");
    //モーターAのピン設定
    MotorPin motorPin[2] = {
        {13, 27, 12}, // モーター右
        {2, 4, 15}  // モーター左
    };
  
}

void loop() {
  if (SerialBT.available()) {
    //文字データフォーマットは方向,速度,モード制御
    String databox = SerialBT.readStringUntil(';');
    Serial.println(databox);

  }
  delay(20);
}