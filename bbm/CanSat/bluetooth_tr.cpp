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
  x_axis = analogRead(read_x);
  y_axis = analogRead(read_y);
  sw     = digitalRead(read_sw);
  //送信用にデータを整形
  String data = String(x_axis) + "," + String(y_axis) + "," + String(sw) + ";";
  
  Serial.print("x=");
  Serial.print(x_axis);
  Serial.print("\ty=");
  Serial.print(y_axis);
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