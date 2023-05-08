#include "sensors.h"

#include "pinsetup.h"

// HardwareSerial Serial_arduino(Serial2);   //サブマイコン用のUARTの番号

Adafruit_BNO055 bno055 = Adafruit_BNO055(-1, 0x28);

#define OPENMV Serial4

void BALL::get() {  // ボールの位置取得
  x = 0;
  y = 0;
  num = 0;
  max = 0;
  maxn = -1;

  for (int i = 0; i < 8; i++) {
    if (i >= 4) {
      digitalWrite(Pin_MPC, 1);
    } else {
      digitalWrite(Pin_MPC, 0);
    }
    if (i % 4 >= 2) {
      digitalWrite(Pin_MPB, 1);
    } else {
      digitalWrite(Pin_MPB, 0);
    }
    if (i % 2 > 0) {
      digitalWrite(Pin_MPA, 1);
    } else {
      digitalWrite(Pin_MPA, 0);
    }
    delayMicroseconds(300);
    value[i]=analogRead(A17);
    value[i+8]=analogRead(A16);
    delayMicroseconds(50);
  }

  // Serial.print(value[0]);
  // Serial.print("  ");
  // Serial.print(value[4]);
  // Serial.print("  ");
  // Serial.print(value[8]);
  // Serial.print("  ");
  // Serial.print(value[12]);

  for (int i = 0; i < NUM_balls; i++) {
    int v = value[i];
    if (v < _th) {
      value[i] = _th - v;
      num++;
      if (value[i] > max) {
        max = value[i];
        maxn = i;
      }
    } else {
      value[i] = 0;
    }
    x = x + (SIN16_1000[i] * ((double)value[i] / 1000.0));
    y = y + (SIN16_1000[(i + 4) % 16] * ((double)value[i] / 1000.0));
  }

  //dir = atan2(x,y) * 57.3;
  distance = sqrt(x * x + y * y);
  if (num >= 1) {
    isExist = true;
    x=0;y=0;
    //for(int i=maxn+15;i<=maxn+17;i++){
    for(int i=0;i<16;i++){
      if(i%2==0 || 1){
      x+=(SIN16_1000[i%16] * ((double)value[i%16] / 1000.0));
      y+=(SIN16_1000[(i + 4) % 16] * ((double)value[i%16] / 1000.0));
      }
    }
    dir = atan2(x,y) * 57.3;
    //Serial.println(dir);
  } else {
    isExist = false;
    dir=1000;
    distance = 1000;
  }

  // if (maxn >= 0) {
  //   dir = maxn * 360 / NUM_balls;
  //   if(dir > 180)dir-=360;
  // } else {
  //   dir = 1000;
  // }

#ifdef ball_debug
if(Serial.read()=='B' || 1){
  Serial.print('B');
  for (int i = 0; i < NUM_balls; i++) {
    Serial.print(value[i] / 4);
    Serial.print(',');
  }
  Serial.print(dir);
  Serial.print(',');
  Serial.print((int)distance);
  Serial.print(',');
  Serial.print('\n');
  // Serial.println("");
  // Serial.print("  ");
  // Serial.print(x);
  // Serial.print("  ");
  // Serial.println(y);
}
#endif
}

void BNO::setup() {
  bno055.begin();
  bno055.getTemp();
  bno055.setExtCrystalUse(true);
  reset();
}

void BNO::get() {
  imu::Vector<3> euler = bno055.getVector(Adafruit_BNO055::VECTOR_EULER);
  ypr[0] = euler.x();
  ypr[1] = euler.y();
  ypr[2] = euler.z();
  dir = ypr[0] - dir0;
  if (dir > 180)
    dir -= 360;
  else if (dir < -179)
    dir += 360;
}

void BNO::reset() {  // 攻め方向リセット
  this->get();
  dir0 = ypr[0];
}

void LINE::get_value() {
  for (int i = 0; i < 8; i++) {
    if (i >= 4) {
      digitalWrite(Pin_MPC, 1);
    } else {
      digitalWrite(Pin_MPC, 0);
    }
    if (i % 4 >= 2) {
      digitalWrite(Pin_MPB, 1);
    } else {
      digitalWrite(Pin_MPB, 0);
    }
    if (i % 2 > 0) {
      digitalWrite(Pin_MPA, 1);
    } else {
      digitalWrite(Pin_MPA, 0);
    }
    delayMicroseconds(500);
    value[i] = analogRead(A0);
    value[i+12] = analogRead(A1);
    value[i+8] = analogRead(A2);
    value[i+4] = analogRead(A3);
    delayMicroseconds(100);
  }
}

void LINE::LEDset(int s = -1) {  // ラインのLED操作
  if (s == -1) {
    s = this->_LED;
  }
  digitalWrite(ledpin, s);
}

int ULTRASONIC::get(int n) {
  // ピンをOUTPUTに設定（パルス送信のため）
  pinMode(trig_pin[n], OUTPUT);
  // LOWパルスを送信
  digitalWrite(trig_pin[n], LOW);
  delayMicroseconds(2);
  // HIGHパルスを送信
  digitalWrite(trig_pin[n], HIGH);
  // 5uSパルスを送信してPingSensorを起動
  delayMicroseconds(5);
  digitalWrite(trig_pin[n], LOW);

  // 入力パルスを読み取るためにデジタルピンをINPUTに変更（シグナルピンを入力に切り替え）
  pinMode(echo_pin[n], INPUT);

  // 入力パルスの長さを測定
  int duration = pulseIn(echo_pin[n], HIGH,timeout);  //応答がなかったら0

  // パルスの長さを半分に分割
  duration = duration / 2;
  // cmに変換
  value[n] = int(duration / 29);
  return value[n];
}

void ULTRASONIC::get_all() {
  for(int i=0;i<4;i++){
    get(i);
  }
}

unsigned long TIMER::get(){
  return millis()-s_tim;
}

void TIMER::reset(){
  s_tim=millis();
}

TIMER openmvtime;

int CAMERA::get_goal(){
  // if(goal_color == OPENMV_YELLOW){
  //   OPENMV.write('y');
  // }else{
  //   OPENMV.write('b');
  // }
  // openmvtime.reset();
  // while(!Serial.available()){
  //   if(openmvtime.get()>OPENMV_timeout){
  //     opp_goal = 1000;
  //     return 1000;
  //   }
  // } // データが来るまで待つ

  // int C=OPENMV.read();

  // if(C==OPENMV_NOTFOUND){
  //   opp_goal = 1000;
  // }else{
  //   opp_goal = C*2;
  // }
  // return opp_goal;

  if(OPENMV.available()){
    return OPENMV.read();
  }
}

void CAMERA::set_color(int c){
  goal_color=c;
}

void CAMERA::begin(){
  OPENMV.begin(9600);
}