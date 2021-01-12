#include <SoftwareSerial.h>
#include <Wire.h>
#include <Servo.h>

#define mA_dir 12 //motor A direction pin
#define mA_speed 3 //motor A PWM (speed) pin
#define mA_brake 9 //motor A brake pin
#define mA_sense A0 //motor A current sensing pin
#define mB_dir 13 //motor B direction pin
#define mB_speed 11 //motor B PWM (speed) pin
#define mB_brake 8 //motor B brake pin
#define mB_sense A1 //motor B current sensing pin
#define m_speed 255

#define esp32_tx 2
#define esp32_rx 4
#define servo_pin 10
#define trig 6
#define echo 7

#define MPU 0x68

float GyroErrorZ = 0, yaw = 0;
float elapsedTime, currentTime = 0, previousTime;

SoftwareSerial esp32(esp32_rx, esp32_tx);
Servo servo;

int servo_pos = 90;

int rev_angle_map(int angle) {
  angle = angle - 90;
  angle = angle * -1;
  return angle;
}

int con_str_int(char c1, char c2, char c3, char i) {
  int a = 0;
  if (i == 3) {
    a = ((c1 - '0')*100) + ((c2 - '0')*10) + (c3 - '0');
  } else if (i == 2) {
    a = ((c1 - '0')*10) + (c2 - '0');
  } else if (i == 1) {
    a = (c1 - '0');
  }
  return a;
}

void setup() {
  pinMode(mA_dir, OUTPUT);
  pinMode(mA_brake, OUTPUT);
  pinMode(mA_speed, OUTPUT);
  pinMode(mB_speed, OUTPUT);
  pinMode(mB_dir, OUTPUT);
  pinMode(mB_brake, OUTPUT);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);

  esp32.begin(9600);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  calculate_IMU_error();

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  
  servo.attach(servo_pin);
  servo.write(servo_pos);
}

void loop() {


  
  delay(1000);
  esp32.print("S");
  bool x = true;
  char c = 10;
  char xx[3];
  char xxn = 0;
  bool xxb = false;
  while (true) {
    if (esp32.available()) {
      if (!xxb && char(esp32.read()) == '_') {xxb=true;continue;}
      if (xxb) {
        xx[xxn++]=char(esp32.read());
        if (xxn == 3) {


          if (xx[0] == 'F' && xx[1] == 'W' && xx[2] == 'D') {
            forward();
          } else if (xx[0] == 'B' && xx[1] == 'W' && xx[2] == 'D') {
            backward();
          } else if (xx[0] == 'L' && xx[1] == 'F' && xx[2] == 'T') {
            rotate_l();
          } else if (xx[0] == 'R' && xx[1] == 'G' && xx[2] == 'T') {
            rotate_r();
          } else if (xx[0] == 'S' && xx[1] == 'T' && xx[2] == 'P') {
            stop_m();
          } else if (xx[0] == 'R' && xx[1] == 'O' && xx[2] == 'L') {
            rotation(float(rev_angle_map(0)), false);
          } else if (xx[0] == 'R' && xx[1] == 'O' && xx[2] == 'R') {
            rotation(float(rev_angle_map(180)), true);
          } else if (xx[0] == 'U' && xx[1] == 'L' && xx[2] == 'T') {
            int us = ultrasonic();
            esp32.print(us);
            esp32.print('_');
          } else if (xx[0] == 'S' && xx[1] == 'R' && xx[2] == 'V') {
            char y[] = {0,0,0};
            char i = 0;
            while (true) {
              if (esp32.available()) {
                char z=char(esp32.read());
                if (z != '_') {
                  y[i] = z;
                  i++;
                } else {
                  int t = con_str_int(y[0], y[1], y[2], i);
                  break;
                }
              }
            }
          }

          
          xxn = 0;
          xx[0] = 0;
          xx[1] = 0;
          xx[2] = 0;
          xxb = false;
        }
      }
      
      
        
    }

  }

 
  
  
 
}

void forward() {
  stop_m();
  digitalWrite(mA_dir, HIGH);
  digitalWrite(mB_dir, HIGH);
  analogWrite(mA_speed, m_speed);
  analogWrite(mB_speed, m_speed);
}

void backward() {
  stop_m();
  digitalWrite(mA_dir, LOW);
  digitalWrite(mB_dir, LOW);
  analogWrite(mA_speed, m_speed);
  analogWrite(mB_speed, m_speed);
}

void rotate_l() {
  stop_m();
  digitalWrite(mA_dir, HIGH);
  digitalWrite(mB_dir, LOW);
  analogWrite(mA_speed, m_speed);
  analogWrite(mB_speed, m_speed);
}

void rotate_r() {
  stop_m();
  digitalWrite(mA_dir, LOW);
  digitalWrite(mB_dir, HIGH);
  analogWrite(mA_speed, m_speed);
  analogWrite(mB_speed, m_speed);
}

void stop_m() {
  analogWrite(mA_speed, 0);
  analogWrite(mB_speed, 0);
  delay(500);
}

void rotation(float angle, bool clockwise) {
  if (clockwise) {
    rotate_r();
    while (yaw >= angle) {
      delay(100);
      get_gyro_angle();
      Serial.println(yaw);
    }
    stop_m();
  } else {
    rotate_l();
    while (yaw <= angle) {
      delay(100);
      get_gyro_angle();
      Serial.println(yaw);
    }
    stop_m();
  }
  yaw = 0;
}

float get_gyro_angle() {
  float GyroZ;
  
  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000;
  
  Wire.beginTransmission(MPU);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 2, true);
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = GyroZ - GyroErrorZ;
  yaw = yaw + GyroZ * elapsedTime;
}

float calculate_IMU_error() {
  float GyroZ;
  int c = 0;
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x47);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 2, true);
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  GyroErrorZ = GyroErrorZ / 200;
}

void set_servo_pos(int pos) {
  if (pos < 0) {
    pos += 180;
  } else if (pos > 179) {
    pos -= 180;
  }

  servo.write(pos);
}

int ultrasonic() {
  long ultra_duration;
  int ultra_distance;
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  ultra_duration = pulseIn(echo, HIGH);
  ultra_distance = ultra_duration * 0.034 / 2;
  return ultra_distance;
}
