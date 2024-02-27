//etoe前に確認すること
//１．溶断のピン番号と時間
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
//それ以外のライブラリ
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <ESP32Servo.h>

//BNO055 setting
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

double currentGPSdata[3] = {0,0,0};
double eulerdata[3] = {0,0,0};
double azidata[2] = {0,0};

//gps setting
//三鷹ファミマの緯度経度
double goalGPSdata2[2] = {35.700993, 139.566498};
//運河駅
double goalGPSdata[2] = {35.914356, 139.905869};
//三鷹駅の緯度経度
double goalGPSdata4[2] = {35.702797, 139.561109};
//ゴールの緯度経度(作業場近くのセブン)
double goalGPSdata3[2] = {35.717147,139.823209};
//I2C communication parameters
#define DEFAULT_DEVICE_ADDRESS 0x42
//I2C read data structures
char buff[80];
int idx = 0;
char lat[9],lon[10];
int PID_left;
int PID_right;

//camera setting
SoftwareSerial mySerial;
String pre_camera_data[3];
int camera_data[3];
double camera_area_data = 0.0;

//dcmoter setting
//define for PID
#define Kp 0.65
const int STBY = 17; // モータードライバの制御の準備
const int AIN1 = 16; // 1つ目のDCモーターの制御
const int AIN2 = 4; // 1つ目のDCモーターの制御
const int BIN1 = 5; // 2つ目のDCモーターの制御
const int BIN2 = 18; // 2つ目のDCモーターの制御
const int PWMA = 0; // 1つ目のDCモーターの回転速度
const int PWMB = 19; // 2つ目のDCモーターの回転速度
const int fusePin = 23;


//collect module setting
const int  SERVO_PIN = 13;
Servo servo;
//左右の回転速度を0基準に設定(v∈[-255,255])
void MoterControl( int left,int right) {
    int absleft = abs(left);
    int absright = abs(right);

    if(left >= 0 && right >= 0){
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMA, absleft);
        analogWrite(PWMB, absright);
    }
    else if(left >= 0 && right < 0){
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        analogWrite(PWMA, absleft);
        analogWrite(PWMB, absright);
    }
    else if(left < 0 && right >= 0){
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
        analogWrite(PWMA, absleft);
        analogWrite(PWMB, absright);
    }
    else{
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        analogWrite(PWMA, absleft);
        analogWrite(PWMB, absright);
    }
}

void stop(){
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
}


//Read 80 bytes from I2C
void readI2C(char *inBuff){
    Wire.requestFrom((uint8_t)DEFAULT_DEVICE_ADDRESS, (uint8_t) 80);
    int i = 0;
    while (Wire.available()) {
        inBuff[i] = Wire.read();
        i++;
    }
}
void GPS_data(){
  
  while(1){
    char c ;
    //改行文字で初期化したい
    if (idx == 0 ) {
      readI2C(buff);
      delay(1);
      //Serial.print("readI2C");
      idx = 0;
    }
    //Fetch the character one by one
    c = buff[idx];
    idx++;
    idx %= 80;
    //If we have a valid character pass it to the library
    if ((uint8_t) c != 0xFF) {
      Serial.print(c);
      //GGAならば緯度経度を取得する
      if (c == '$' && idx < 40) {
        if(buff[idx+2] == 'G'){
          if(buff[idx+3] == 'G'){
            if(buff[idx+4] == 'A'){
                for(int i = 0; i < 4; i++){
                    lat[i] = buff[idx+16+i];
                }
                for(int i = 0; i < 5; i++){
                    //小数点は除外する
                    lat[i + 4] = buff[idx+21+i];
                }
                for(int i = 0; i < 5; i++){
                    //小数点は除外する
                    lon[i] = buff[idx+29+i];
                }
                for(int i = 0; i < 5; i++){
                    //小数点は除外する
                    lon[i + 5] = buff[idx+35+i];
                }
                long mlat = atol(lat);
                double latitude = (double)mlat * 1.0E-7;
                long mlon = atol(lon);
                double longitude = mlon * 1.0E-7;
                double goaldirection  = 57.2957795131 * atan2(goalGPSdata[0] - latitude, goalGPSdata[1] - longitude);
                if(goaldirection > -90){
                    goaldirection -= 90;
                }else{
                    goaldirection += 270;
                }
                Serial.print("latitude: ");
                Serial.println(latitude);
                Serial.print("\tlongitude: ");
                Serial.println(longitude);
                delay(10);
                currentGPSdata[0] = latitude;
                currentGPSdata[1] = longitude;
                currentGPSdata[2] = goaldirection;
                break;
              
            }
          }
        }
      }
    
    }
  }
}

void Euler(){
    
    imu::Quaternion quat = bno.getQuat();
    double w = quat.w();
    double x = quat.x();
    double y = quat.y();
    double z = quat.z();

    double ysqr = y * y;

    // roll (x-axis rotation)
    double t0 = +2.0 * (w * x + y * z);
    double t1 = +1.0 - 2.0 * (x * x + ysqr);
    double roll = atan2(t0, t1);

    // pitch (y-axis rotation)
    double t2 = +2.0 * (w * y - z * x);
    t2 = t2 > 1.0 ? 1.0 : t2;
    t2 = t2 < -1.0 ? -1.0 : t2;
    double pitch = asin(t2);

    // yaw (z-axis rotation)
    double t3 = +2.0 * (w * z + x * y);
    double t4 = +1.0 - 2.0 * (ysqr + z * z);  
    double yaw = atan2(t3, t4);

    //ラジアンから度に変換
    roll *= 57.2957795131;
    pitch *= 57.2957795131;
    yaw *= 57.2957795131;

    eulerdata[0] = roll;
    eulerdata[1] = pitch;
    eulerdata[2] = yaw;
}
double distanceBetween(double lat1, double long1, double lat2, double long2){
    // returns distance in meters between two positions, both specified
    // as signed decimal-degrees latitude and longitude. Uses great-circle
    // distance computation for hypothetical sphere of radius 6372795 meters.
    // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
    // Courtesy of Maarten Lamers
    double delta = radians(long1-long2);
    double sdlong = sin(delta);
    double cdlong = cos(delta);
    lat1 = radians(lat1);
    lat2 = radians(lat2);
    double slat1 = sin(lat1);
    double clat1 = cos(lat1);
    double slat2 = sin(lat2);
    double clat2 = cos(lat2);
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = sq(delta);
    delta += sq(clat2 * sdlong);
    delta = sqrt(delta);
    double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);
    return delta * 6372795;
}

void GetAzimuthDistance(){
    GPS_data();
    Euler();
    //回転の程度をとりあえず整えてみる(turnpower∈[-180,180])
    double turnpower;
    turnpower = currentGPSdata[2] - eulerdata[2];
    if (turnpower > 180){
        turnpower -= 360;
    }
    else if (turnpower < -180){
        turnpower += 360;
    }
    Serial.print("GPS Data : ");
    Serial.print(currentGPSdata[2]);
    Serial.print("\tEuler Data: ");
    Serial.print(eulerdata[2]);
    Serial.print("\tMoterpower : ");
    Serial.println(turnpower);
    
    azidata[0] = turnpower;
    azidata[1] = distanceBetween(goalGPSdata[0],goalGPSdata[1],currentGPSdata[0],currentGPSdata[1]);
    Serial.print("\tDistance: ");
    Serial.println(azidata[1]);
}
//GPSとオイラー角から右回転を正として回転量を出す
void P_GPS_Moter(){ 
    Serial.println("P_GPS_Moter");
    while(true){
    GetAzimuthDistance();
    if(azidata[1] < 5){
        break;
        }
    else{
        if(azidata[0] > 0){
            PID_left = 240;
            PID_right = 240 - Kp * azidata[0];
        }
        else{
            PID_right = 240;
            PID_left = 240 + Kp * azidata[0];
        }｝
        Serial.print("\tMoterpower : ");
        Serial.print(PID_left);
        Serial.println(",");
        Serial.println(PID_right);
        MoterControl(PID_left, PID_right);
        delay(250);
        }
    } 
}
void housyutu(){
    Serial.println("housyutu");
    while (1){
        Euler();
        Serial.println(eulerdata[1]);
        if (eulerdata[1] <= 45 && eulerdata[1] >= -45) {
            delay(10000);
            Serial.println("youdan");
            digitalWrite(fusePin, HIGH); // 溶断回路を通電
            delay(5000);
            digitalWrite(fusePin, LOW); // 
            break;
            }
        else{
            delay(1000);
            }
    }
}
void split(String data){
    int index = 0; 
    int datalength = data.length();
    bool startParsing = false;
    //文字列データの初期化
    pre_camera_data[0] = "";
    pre_camera_data[1] = "";
    for (int i = 0; i < 15; i++) {
        char tmp = data.charAt(i);
        if (tmp == '$') {
            startParsing = true;
            continue; // Skip processing the '$' character
        }
        
        if (startParsing) {
            //画角に赤が写っていない場合の例外処理
            if((index == 0) && (tmp == '0')){
                pre_camera_data[0] = "0";
                pre_camera_data[1] = "0";
                pre_camera_data[2] = "0.0";
                break;
            }
            if (tmp == ',') {
                index++;
            } else {
                pre_camera_data[index] += tmp;
            }

        }
    }
    //文字列リストを整数リストに変換
    for(int i = 0; i < 2; i++){
        camera_data[i] = pre_camera_data[i].toInt();
    }
    camera_area_data = pre_camera_data[2].toDouble();
    
    
}
void P_camera_Moter(){
    char buff[255];
    int counter = 0;
    while(1){//カメラによる制御のためのループ
        while (mySerial.available()) { // 同期のためにデータをすべて一旦破棄する
            mySerial.flush();
        }
        delay(1);
        if(mySerial.available()>0){
            char val = char(mySerial.read());
            if (val == '$') {
                buff[0] = '$';
                counter++; 
                while(1){//カメラからのシリアル通信によるデータを受け取るためのループ、あとでデータが読めなかった場合の例外を追加する
                    if(mySerial.available()>0){
                        char nextval = char(mySerial.read());
                        buff[counter] = nextval;//x軸、y軸、面積のデータを格納
                        counter++;  
                        //二個目のメッセージであるかを判断
                        if (counter == 15){
                            Serial.println(buff);
                            //文字列を整数リストに変換
                            split(buff);
                            if(camera_area_data>0.40){
                                break;
                            }
                            Serial.print(camera_data[0]);delay(10);//シリアルモニタの表示がバグるので時間を置く、ここが変
                            Serial.print(",");delay(10);
                            Serial.println(camera_data[1]);delay(10);
                            int PID2_left = 0.75 * (camera_data[0]-160) + 120;
                            int PID2_right = 0.75 * (160 - camera_data[0]) + 120;
                            MoterControl(PID2_left,PID2_right);
                            Serial.print("\tMoterControl: ");delay(10);
                            Serial.print(PID2_left);delay(10);
                            Serial.print(",");delay(10);
                            Serial.println(PID2_right);delay(10);

                            //MoterControl(PID2_left,PID2_right);
                            delay(1000);
                            counter = 0;
                            break;
                        }
                    }
                }
                if(camera_area_data>0.40){//二重ループを抜けるための応急措置
                    break;
                }
            }
        }
    }
}
void setup() {
    //serial setting
    Serial.begin(57600);
    // 速度、RX、TX、?、?、バッファ
    mySerial.begin(57600,32, 33,SWSERIAL_8N1,false,256);
    Serial.println("Starting ...");
    
    //i2c setting
    Wire.begin(21,22);
    delay(100);

    //moter setting
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    digitalWrite(STBY, HIGH);
    // スタンバイ
    pinMode(STBY, OUTPUT);
    pinMode(PWMA, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(fusePin, OUTPUT);
    digitalWrite(fusePin, LOW);

    // servo setting
    servo.attach(SERVO_PIN,510,2400);
    //BNO055関連
    delay(1000);
    bool status = bno.begin();
    if (!bno.begin()){
        /* There was a problem detecting the BNO055 ... check your connections */
        Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
        while (1);
    }
    delay(100);

    //clear i2c buffer
    char c;
    idx = 0;
    memset(buff, 0, 80);
    do {
    if (idx == 0) {
        readI2C(buff);
        delay(1);
    }
    c = buff[idx];
    idx++;
    idx %= 80;
    }
    while ((uint8_t) c != 0xFF);
}

void loop() {
    //release sequence
    Serial.println("release sequence start");
    delay(100);
    //housyutu();
    //go straight
    MoterControl(200,200);
    delay(1000);
    stop();
    P_GPS_Moter();
    //アーム展開
    Serial.println("camera sequence start");
    P_camera_Moter();
    //delay(1000);
    delay(9999999);
    // put your main code here, to run repeatedly:
}