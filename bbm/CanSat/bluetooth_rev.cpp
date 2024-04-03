#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
//bluetooth ready
#include "BluetoothSerial.h"

typedef struct{
    int in1;
    int in2;
    int pwm;
} MotorPin;

//モーターAのピン設定
    MotorPin motorPin[2] = {
        {13, 27, 12}, // モーター右
        {2, 4, 15}  // モーター左
    };

BluetoothSerial SerialBT;

String pre_rev_data[3]; //受信データの一時保存用
int rev_data[3] = {0,0,0};




void MotorOut(MotorPin pin[], int speed, int omega){
    //モーターの速度調整
    //両方の値が大きい時に角度に依存しているため、調整が必要
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
    MotorOut(motorPin, 255, 0);
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
    //文字列データを数値に変換
    for (int i = 0; i < 3; i++) {
        rev_data[i] = pre_rev_data[i].toInt();
    }
    //データの初期化
    for (int i = 0; i < 3; i++) {
        pre_rev_data[i] = "";
    }
}

void setup() {
    Serial.begin(115200);
    SerialBT.begin("sinkan");
    Serial.println("device start");
    
    // ピンを出力モードに設定
    for(int i = 0; i < 2; i++){
        pinMode(motorPin[i].in1, OUTPUT);
        pinMode(motorPin[i].in2, OUTPUT);
        pinMode(motorPin[i].pwm, OUTPUT);
    }
}

void loop() {
  if (SerialBT.available()) {
    //文字データフォーマットは回転,速度,モード制御
    String databox = SerialBT.readStringUntil(';');
    receiveData(databox);
    Serial.println(databox);
    Serial.print("rev_data[0]:");
    Serial.println(rev_data[0]);
    Serial.print("rev_data[1]:");
    Serial.println(rev_data[1]);
    Serial.print("rev_data[2]:");
    Serial.println(rev_data[2]);
    MotorOut(motorPin, rev_data[1], rev_data[0]);
    delay(10);
  }
}