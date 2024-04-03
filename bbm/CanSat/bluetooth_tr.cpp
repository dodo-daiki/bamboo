//servo check
//モーターの右左
//キャリブレーションの正確性
//etoe前に確認すること
//１．溶断のピン番号と時間S
//sdcardの中身消す、書き込み練習
//microsd
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
//bluetooth ready
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
uint8_t address[6]  = {0x08, 0xB6, 0x1F, 0xEE, 0x42, 0x2A};
bool connected;
#define read_x  4                         //X軸(横軸)
#define read_y  0                         //7軸(縦軸)
#define read_sw 2                         //スイッチ

int x_axis, y_axis, sw;
int sokudo, kaiten;

void setup() {
  //serial ready
  Serial.begin(115200);
  SerialBT.begin("sinkan", true); 
  Serial.println("device start");
  connected = SerialBT.connect(address);
  if(connected) {
    Serial.println("Connect OK");
  } else {
    while(!SerialBT.connected(10000)) {
      Serial.println("No connect"); 
    }
  }
  
  if (SerialBT.disconnect()) {
    Serial.println("Disconnected Succesfully!");
  }

  SerialBT.connect();
  delay(100);
  pinMode(read_x, INPUT);
  pinMode(read_y, INPUT);
  pinMode(read_sw, INPUT_PULLUP);
}

void loop() {
  //値の範囲は0~4095
  x_axis = analogRead(read_x);
  y_axis = analogRead(read_y);
  Serial.print("x=");
  Serial.print(x_axis);
  Serial.print("\ty=");
  Serial.print(y_axis);
  //送信用に修正
  //xの値をもとに回転度合いを変更
  //ジョイスティックを動かしてない状態での値を0にする
  if(x_axis > 2100 && y_axis > 2100){
    sokudo = 0;
    kaiten = 0;
  }else{
    //[0,2800]
    x_axis = (-x_axis + 2500) / 10;
    y_axis = (-y_axis + 2500) / 10;
    sokudo = sqrt(x_axis * x_axis + y_axis * y_axis) * 200 / 350;
    //最大値調整
    if(sokudo > 255){
      sokudo = 255;
    }
    //回転を-255~255に変換
    kaiten = atan2(y_axis, x_axis) * 180 / PI;
    kaiten = kaiten - 45;
    kaiten = kaiten * 255 / 45;
    if(kaiten > 255){
      kaiten = 255;
    }
    if(kaiten < -255){
      kaiten = -255;
    }


  }

  //押してるときに0、押してないときに1
  sw     = digitalRead(read_sw);
  if(sw == 1){                //スイッチを離したとき
    sw     =0;
  }else{                     //スイッチを押したとき
    sw     =1;
  }
  //送信用にデータを整形
  String data = String(kaiten) + "," + String(sokudo) + "," + String(sw) + ";";
  
  Serial.print("sokudo=");
  Serial.print(sokudo);
  Serial.print("\tkaiten=");
  Serial.print(kaiten);
  Serial.print("\tswitch");
  if(sw == 1){                //スイッチを離したとき
    Serial.println("OFF");
  }else{                     //スイッチを押したとき
    Serial.println("ON");
  }
  SerialBT.print(data);
  
/*
   for(int i = 0; i < 10; i++){
      SerialBT.write('T');
    }
    SerialBT.write(';');
*/  
  delay(500);
}