#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <move.h>
#include <pinsetup.h>
#include <sensors.h>



#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define SCREEN_ADDRESS \
  0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, -1);

#define buzzer 30

MOTOR motor;
BALL ball;
BNO gyro;
ULTRASONIC ping;
MOVE move;
CAMERA openmv;

TIMER timer[20];
TIMER linetim;
TIMER pingset;

TIMER timfps;
int fps=0;

int z = -1;  // ラインの戻る方向
int dir_move;
bool kick = 0;

int hold_th;

void sensormonitor();
int mode = 0;  // センサーモニターモード

int line_th[32];

void setup() {
  //ピン設定
  pinMode(Pin_MPA, OUTPUT);
  pinMode(Pin_MPB, OUTPUT);
  pinMode(Pin_MPC, OUTPUT);

  pinMode(Pin_kicker, OUTPUT);

  pinMode(A21, INPUT);
  pinMode(A22, INPUT);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  pinMode(A16, INPUT);
  pinMode(A17, INPUT);

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);

  pinMode(13, OUTPUT);

  pinMode(Pin_S1, INPUT);
  pinMode(Pin_S2, INPUT);
  pinMode(Pin_S3, INPUT);
  pinMode(Pin_TS, INPUT);

  pinMode(Pin_ballcatch, INPUT);

  pinMode(buzzer, OUTPUT);
  tone(buzzer, 2714, 120);
  openmv.begin();
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.println("Singularity");
  display.display();
  

  Serial.begin(9600);

  motor.stop();
  motor.pwm_out();

  gyro.setup();



  for (int i = 0; i < 32; i++) {
    line_th[i] = EEPROM.read(i) * 4;
  }
  //EEPROM.write(34, 100);
  hold_th = EEPROM.read(34);

  timer[0].reset();
  tone(buzzer, 2714, 120);
  int i = 0;
  while (timer[0].get() < 2100) {
    if (SW1 || SW2 || SW3) {
      noTone(buzzer);
      tone(buzzer, 2500,100);
      break;
    }
    if (timer[0].get() > 1400 && i == 2) {
      noTone(buzzer);
      tone(buzzer, 3047);
      i++;
    } else if (timer[0].get() > 700 && i == 1) {
      noTone(buzzer);
      tone(buzzer, 2876);
      i++;
    } else if (i == 0) {
      noTone(buzzer);
      tone(buzzer, 2714);
      i++;
    }
  }
  noTone(buzzer);
  while (SW1 || SW2 || SW3)
    ;
}

void loop() {
  fps++;
  if(timfps.get()>1000){
    display.clearDisplay();
    display.setCursor(40,25);
    display.setTextSize(4);
    display.print(fps);
    display.display();
    fps=0;
    timfps.reset();
  }
  sensormonitor();
  int line[4][8] = {0};
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
    line[0][i] = analogRead(A0);
    line[1][i] = analogRead(A1);
    line[2][i] = analogRead(A2);
    line[3][i] = analogRead(A3);
    delayMicroseconds(50);
  }

  int line_s[4] = {0};
  int line_n[4] = {0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      // if ((line[i][j] > 500 && j != 3) || line[i][j] > 850) {
      //   line_s[i] = 1;
      // } else if (j < 2 && line[i][j] > 230) {
      //   line_s[i] = 1;
      // }

      // if (i == 0 && 1) {
      //   Serial.print(line[i][j]);
      //   Serial.print("  ");
      // }
      if (line[i][j] > line_th[i * 8 + j]) {
        line_s[i] = 1;
        if (j >= 5) {
          line_n[i] = 1;
        }
      }
    }
    // Serial.print(line_s[i]);
  }
  // for (int i = 1; i < 4; i++) {
  //   ping.get(i);
  //   // Serial.print(ping.value[i]);
  //   // Serial.print("  ");
  // }
  // Serial.println();

  // display.clearDisplay();
  // int line_c[12] = {line[0][6], line[0][7], line[3][5], line[3][6],
  //                   line[3][7], line[2][5], line[2][6], line[2][7],
  //                   line[1][5], line[1][6], line[1][7], line[0][5]};

  // for (int i = 0; i < 12; i++) {
  //   if (line_c[i] > 400) {
  //     display.fillRect(64 + cos(radians(i * 360 / 12)) * 25 - 3,
  //                      32 + sin(radians(i * 360 / 12)) * 25 - 3, 6, 6,
  //                      SSD1306_WHITE);
  //   } else {
  //     display.drawRect(64 + cos(radians(i * 360 / 12)) * 25 - 1,
  //                      32 + sin(radians(i * 360 / 12)) * 25 - 1, 2, 2,
  //                      SSD1306_WHITE);
  //   }
  //}
  // display.display();

  if (pingset.get() > 300) {
    ping.get(1);
    pingset.reset();
  }

  if (line_s[0] + line_s[1] + line_s[2] + line_s[3]) {
    // tone(buzzer, 2000);
  } else {
    noTone(buzzer);
  }

  // if (SW3) {
  //   gyro.reset();
  //   tone(buzzer, random(1000, 4000), 100);
  //   while (SW3)
  //     ;
  // }

  ball.get();
  gyro.get();

  if (ball.isExist) { //ボール見えた
    // int z;
    // if (ball.dir < 15 && ball.dir > -15) {
    //   dir_move = 0;
    // } else if (ball.dir > 0) {
    //   dir_move = ball.dir + 50;
    // } else {
    //   dir_move = ball.dir - 50;
    // }
    move.carryball(ball.dir);
    dir_move=move.dir;
    if (dir_move > 180)
      dir_move -= 360;
    else if (dir_move < -180)
      dir_move += 360;
    // motor.cal_power(dir_move, 60, -gyro.dir / 2);

    timer[13].reset();
  } else {
    dir_move = 1000;
    // motor.cal_power(0, 0, -gyro.dir / 2);
  }


  if(timer[13].get()>500){
    ping.get(1);
    ping.get(2);
    if(ping.value[2] > 80){
      if(ping.value[1] < 60){ //右より
        dir_move = -135;
      }else if(ping.value[1] > 100){
        dir_move = 135;
      }else{
        dir_move = 180;
      }
    }else{
      if(ping.value[1] < 60){ //右より
        dir_move = -90;
      }else if(ping.value[1] > 100){
        dir_move = 90;
      }else{
        dir_move = 1000;
      }
    }
  }

  // if(ball1<ball2-100){
  //   motor.cal_power(90, 60, -gyro.dir / 2);
  // }else if(ball2<ball1-100){
  //   motor.cal_power(-90, 60, -gyro.dir / 2);
  // }else{
  //   motor.stop();
  // }
  int speed = 80;
  if (dir_move != 1000) {
    if (line_s[0]) {
      if (dir_move < 90 && dir_move > -90) {
        speed = 80;
        if (dir_move < 30 && dir_move > -30) {
          dir_move = 1000;
        } else if (dir_move > 0) {
          dir_move = 90;
        } else {
          dir_move = -90;
        }
      }
    }
    if (line_s[1]) {
      speed = 80;
      if (dir_move < 0 && dir_move > -180) {
        if(ball.dir + gyro.dir > 0){
          dir_move = ball.dir;
        }else if (dir_move > -120 && dir_move < -60) {
          dir_move = 1000;
        } else if (dir_move > -90) {
          dir_move = 0;
        } else {
          dir_move = -180;
        }
      }
    }
    if (line_s[2]) {
      if (dir_move > 90 || dir_move < -90) {
        speed = 80;
        // if (dir_move > 150 || dir_move < -150) {
        //   dir_move = 1000;
        // } else if (dir_move > 0) {
        //   dir_move = 90;
        // } else {
        //   dir_move = -90;
        // }
        if(ball.dir < 90 && ball.dir > -90){
          if(dir_move>0){
            dir_move = 90;
          }else{
            dir_move = -90;
          }
        }else if (dir_move > 150 || dir_move < -150) {
          dir_move = 1000;
        } else if (dir_move > 0) {
          dir_move = 90;
        } else {
          dir_move = -90;
        }
        
      }
    }
    if (line_s[3]) {
      if (dir_move > 0 && dir_move < 180) {
        speed = 80;
        if(ball.dir + gyro.dir < 0){
          dir_move = ball.dir;
        }else if (dir_move < 120 && dir_move > 60) {
          dir_move = 1000;
        } else if (dir_move < 90) {
          dir_move = 0;
        } else {
          dir_move = -180;
        }
      }
    }
  }

  if (z == -1) {
    if (line_n[0]) {
      linetim.reset();
      z = 180;
    } else if (line_n[1]) {
      linetim.reset();
      z = 90;
    } else if (line_n[2]) {
      linetim.reset();
      z = 0;
    } else if (line_n[3]) {
      linetim.reset();
      z = -90;
    }
  }
  if (analogRead(Pin_ballcatch) < hold_th) {
    timer[11].reset();
  }
  int kickdir = 0;
  if (timer[11].get() < 500 && ball.isExist) {  // ボール捕捉時
    if (ping.value[1] < 60) {        // ゴールは左
      motor.cal_power(dir_move, speed, -(gyro.dir + 45) / 2);
      kickdir = -45;
    } else if (ping.value[1] > 100) {
      motor.cal_power(dir_move, speed, -(gyro.dir - 45) / 2);
      kickdir = 45;
    } else
      motor.cal_power(dir_move, speed, -(gyro.dir) / 2);
  } else {
    motor.cal_power(dir_move, speed, -(gyro.dir) / 2);
  }

  // if(timer[0].get()<1300){
  //   if(timer[0].get()<300)dir_move=180;
  //   motor.cal_power(dir_move, speed, -(gyro.dir-55) / 2);
  //   tone(buzzer,500);
  // }
  if (z != -1 && TS) {
    motor.cal_power(z, 80, -gyro.dir / 2);
    motor.pwm_out();
    // tone(buzzer, 2000, 100);
  }
  if(line_s[0] || line_s[1] || line_s[2] || line_s[3]){ //どれかのセンサーが反応
    timer[16].reset();
  }
  if (linetim.get() > 200) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3]) z = -1;
  }
  if (z == 0 && linetim.get() > 20) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[0]) {
      z = -1;
    }
  }
  if (z == 1 && linetim.get() > 20) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[1]) {
      z = -1;
    }
  }
  if (z == 2 && linetim.get() > 20) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[2]) {
      z = -1;
    }
  }
  if (z == 3 && linetim.get() > 20) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[3]) {
      z = -1;
    }
  }

  if (!TS) {
    motor.stop();
  }

  if (analogRead(Pin_ballcatch) > hold_th) {
    timer[7].reset();
  } else {
    // if(line_s[0]){ //角判定
    //   timer[0].reset();
    // }
  }
  if (timer[7].get() > 100 && !kick && timer[9].get() > 600 &&
      abs(gyro.dir - kickdir) < 10) {
    kicker(1);
    timer[8].reset();
    kick = 1;
  }

  if (timer[8].get() > 100 && kick) {
    kicker(0);
    timer[7].reset();
    kick = 0;
    timer[9].reset();
  }
  if (gyro.ypr[2] < -5) {
    motor.stop();
    digitalWrite(13, LOW);
  } else {
    digitalWrite(13, HIGH);
  }

  motor.pwm_out();

  // tone(buzzer,ball.value[0]*5);
}

#define MODE_MAX 5

int lp = 0;
void sensormonitor() {
  display.setTextSize(2);
  if(!TS){
  while (!TS) {
    //digitalWrite(13, HIGH);
    gyro.get();
    if (gyro.ypr[2] < -5 && mode!=1) {
      digitalWrite(13, LOW);
    } else {
      digitalWrite(13, HIGH);
    }
    display.setRotation(0);
    motor.stop();
    motor.pwm_out();
    kicker(0);

    if (SW1) {
      tone(buzzer, 1077, 100);
      mode++;
      if (mode > MODE_MAX) {
        mode = 0;
      }
      delay(100);
      while (SW1)
        ;
    }
    if (SW3) {
      tone(buzzer, 2000, 100);
      gyro.reset();
      while (SW3)
        ;
    }

    if (mode == 0) {  // ボールモニター
      ball.get();
      display.clearDisplay();
      int SCALE = 15;
      for (int i = 0; i < 16; i++) {
        display.drawLine(
            cos(i * 2 * PI / 16) * ball.value[i] / SCALE + 64,
            sin(i * 2 * PI / 16) * ball.value[i] / SCALE + 32,
            cos((i + 1) * 2 * PI / 16) * ball.value[(i + 1) % 16] / SCALE + 64,
            sin((i + 1) * 2 * PI / 16) * ball.value[(i + 1) % 16] / SCALE + 32,
            SSD1306_WHITE);
      }
      if (ball.isExist)
        display.drawLine(64, 32, -sin(radians(ball.dir - 90)) * 25 + 64,
                         cos(radians(ball.dir - 90)) * 25 + 32, SSD1306_WHITE);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println(ball.dir);
      display.display();
    } else if (mode == 1) {  // ラインモニター
      display.clearDisplay();
      int line[4][8] = {0};
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
        line[0][i] = analogRead(A0);
        line[1][i] = analogRead(A1);
        line[2][i] = analogRead(A2);
        line[3][i] = analogRead(A3);
        delayMicroseconds(100);
      }
      int line_m[4][4] = {0};
      for (int i = 0; i < 4; i++) {
        if (line[i][2] > line_th[i * 8 + 2]) {
          line_m[i][0] = 1;
        }
        if (line[i][3] > line_th[i * 8 + 3]) {
          line_m[i][1] = 1;
        }
        if (line[i][4] > line_th[i * 8 + 4]) {
          line_m[i][2] = 1;
        }
        if (line[i][5] > line_th[i * 8 + 5]) {
          line_m[i][3] = 1;
        }
      }

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          if (line_m[i][j] > 0) {
            display.fillRect(64 + cos(i * PI / 2) * (5 - j) * 6 - 3,
                             32 - sin(i * PI / 2) * (5 - j) * 6 - 3, 6, 6,
                             SSD1306_WHITE);
          } else {
            display.fillRect(64 + cos(i * PI / 2) * (5 - j) * 6 - 1,
                             32 - sin(i * PI / 2) * (5 - j) * 6 - 1, 2, 2,
                             SSD1306_WHITE);
          }
          if (lp == i * 4 + j) {
            display.drawCircle(64 + cos(i * PI / 2) * (5 - j) * 6,
                               32 - sin(i * PI / 2) * (5 - j) * 6, 7,
                               SSD1306_WHITE);
          }
        }
        if (line[i][1] > line_th[i * 8 +1]) {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 + sin(i * PI / 2) * 6 - 3,
              32 - sin(i * PI / 2) * (5 - 0) * 6 + cos(i * PI / 2) * 6 - 3, 6,
              6, SSD1306_WHITE);
        } else {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 + sin(i * PI / 2) * 6 - 1,
              32 - sin(i * PI / 2) * (5 - 0) * 6 + cos(i * PI / 2) * 6 - 1, 2,
              2, SSD1306_WHITE);
        }
        if (line[i][0] > line_th[i * 8 + 0]) {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 - sin(i * PI / 2) * 6 - 3,
              32 - sin(i * PI / 2) * (5 - 0) * 6 - cos(i * PI / 2) * 6 - 3, 6,
              6, SSD1306_WHITE);
        } else {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 - sin(i * PI / 2) * 6 - 1,
              32 - sin(i * PI / 2) * (5 - 0) * 6 - cos(i * PI / 2) * 6 - 1, 2,
              2, SSD1306_WHITE);
        }

        if (line[i][6] > line_th[i * 8 + 6]) {
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 3,
                           32 - sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 1,
                           32 - sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
        if (line[i][7] > line_th[i * 8 + 7]) {
          display.fillRect(64 + cos((i / 2.0 - 1.0 / 6.0) * PI) * 12 - 3,
                           32 - sin((i / 2.0 - 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 - 1.0 / 6.0) * PI) * 12 - 1,
                           32 - sin((i / 2.0 - 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
      }
      display.setCursor(0, 0);
      display.print(line[lp / 4][lp % 4 + 2]);
      if (SW2) {
        lp++;
        if (lp > 15) lp = 0;
        delay(50);
        tone(buzzer, 2714, 10);
        while (SW2)
          ;
      }
      display.display();
    } else if (mode == 2) {
      display.clearDisplay();
      display.setCursor(0, 0);
      // display.print(analogRead(Pin_ballcatch));
      display.println("Gyro");
      
      display.println(gyro.dir);
      // display.println(gyro.ypr[1]);
      // display.println(gyro.ypr[2]);
      display.display();
    } else if (mode == 3) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("line set");
      display.display();

      if (SW2) {
        int min[32];
        int max[32] = {0};
        for (int i = 0; i < 32; i++) {
          min[i] = 1024;
        }
        while (SW2) {
          tone(buzzer, 2000);
          int line[4][8] = {0};
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
            line[0][i] = analogRead(A0);
            line[1][i] = analogRead(A1);
            line[2][i] = analogRead(A2);
            line[3][i] = analogRead(A3);
            delayMicroseconds(100);

            if (line[0][i] > max[i]) max[i] = line[0][i];
            if (line[1][i] > max[i + 8]) max[i + 8] = line[1][i];
            if (line[2][i] > max[i + 16]) max[i + 16] = line[2][i];
            if (line[3][i] > max[i + 24]) max[i + 24] = line[3][i];
            if (line[0][i] < min[i]) min[i] = line[0][i];
            if (line[1][i] < min[i + 8]) min[i + 8] = line[1][i];
            if (line[2][i] < min[i + 16]) min[i + 16] = line[2][i];
            if (line[3][i] < min[i + 24]) min[i + 24] = line[3][i];
          }
        }
        if (SW3) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.print("EEPROM writeing...");
          display.display();
          tone(buzzer, 2500, 800);
          delay(1000);
          tone(buzzer, 3000, 1000);
          for (int i = 0; i < 32; i++) {
            //EEPROM.write(i, (max[i] + min[i]) / 8);
            EEPROM.write(i,(((max[i]-min[i])/4 + min[i]) /4));
          }
          for (int i = 0; i < 32; i++) {
            line_th[i] = EEPROM.read(i) * 4;
          }
        }
      }
    } else if (mode == 4) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("PING");

      for (int i = 1; i < 4; i++) {
        ping.get(i);
        display.println(ping.value[i]);
      }
      display.display();

    } else if (mode == 5) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("CAMERA");

      display.println("BLUE");

      openmv.set_color(OPENMV_BLUE);
      display.println(openmv.get_goal());
      display.display();

      if(SW2){
        kicker(1);
        delay(100);
        kicker(0);
        delay(200);
        
      }

    }  // end if
  }    // end while
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.println("Singularity");
  display.display();
  }
}


