#include <SoftwareSerial.h>
#include <Wire.h>
#include <IRremote.h>
#include <Servo.h>

#define mA_dir 12 //motor A direction pin
#define mA_speed 3 //motor A PWM (speed) pin
#define mA_brake 9 //motor A brake pin
#define mA_sense A0 //motor A current sensing pin
#define mB_dir 13 //motor B direction pin
#define mB_speed 11 //motor B PWM (speed) pin
#define mB_brake 8 //motor B brake pin
#define mB_sense A1 //motor B current sensing pin
#define m_speed 123

#define esp32_tx 2
#define esp32_rx 4
#define ir 5
#define servo_pin 10
#define trig 6
#define echo 7

#define MPU 0x68

float GyroErrorZ = 0, yaw = 0;
float elapsedTime, currentTime = 0, previousTime;

SoftwareSerial esp32(esp32_rx, esp32_tx);
IRrecv remote(ir);
Servo servo;

decode_results results;
int servo_pos = 0;
unsigned long remote_value = 0;
long ultra_duration;
int ultra_distance;

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
  Serial.begin(9600);
  while (!Serial) {
    
  }
  
  Serial.println("Initialising...");
  
//  pinMode(mA_dir, OUTPUT);
//  pinMode(mA_brake, OUTPUT);
//  pinMode(mB_dir, OUTPUT);
//  pinMode(mB_brake, OUTPUT);

  esp32.begin(9600);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  calculate_IMU_error();
  Serial.print("IMU Error: ");
  Serial.println(GyroErrorZ);

  remote.enableIRIn();
  remote.blink13(true);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  
  //servo.attach(servo_pin);
  //servo.write(servo_pos);

  Serial.println("Initialised");
}

void loop() {
  /*forward();
  delay(3000);
  stop();
  delay(1500);
  rotate_l();
  delay(3000);
  stop();
  delay(1500);*/
  delay(1000);
  Serial.println("sending");
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
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'B' && xx[1] == 'W' && xx[2] == 'D') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'L' && xx[1] == 'F' && xx[2] == 'T') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'R' && xx[1] == 'G' && xx[2] == 'T') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'S' && xx[1] == 'T' && xx[2] == 'P') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'R' && xx[1] == 'O' && xx[2] == 'L') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'R' && xx[1] == 'O' && xx[2] == 'R') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
          } else if (xx[0] == 'U' && xx[1] == 'L' && xx[2] == 'T') {
            Serial.print(xx[0]);Serial.print(xx[1]);Serial.println(xx[2]);
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
                  Serial.println(t);
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

  /*
  get_gyro_angle();
  Serial.println(yaw);
  delay(1000);

  if (remote.decode(&results)) {
    remote.resume();
    delay(200);
    if (results.value == remote_value) {
      remote_value = 0;
      Serial.println("STP");
    } else {
      switch (results.value) {
        case 16718055:
          Serial.println("FWD");
          remote_value = results.value;
          break;
        case 16730805:
          Serial.println("BWD");
          remote_value = results.value;
          break;
        case 16716015:
          Serial.println("LFT");
          remote_value = results.value;
          break;
        case 16734885:
          Serial.println("RGT");
          remote_value = results.value;
          break;
      }
    }
    //Serial.print("Code: ");
    //Serial.println(results.value);
  }

  for(servo_pos = 0; servo_pos < 180; servo_pos += 10) {
    set_servo_pos(servo_pos);
    delay(15);
  }
  
  for(servo_pos = 180; servo_pos >= 1; servo_pos -= 10) {
    set_servo_pos(servo_pos);
    delay(15);
  }

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  ultra_duration = pulseIn(echo, HIGH);
  ultra_distance = ultra_duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(ultra_distance);
  Serial.println(" cm");
  delay(1000);*/
}

void forward() {
  stop_m();
  digitalWrite(mA_dir, HIGH);
  digitalWrite(mB_dir, LOW);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void backward() {
  stop_m();
  digitalWrite(mA_dir, LOW);
  digitalWrite(mB_dir, HIGH);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void rotate_l() {
  stop_m();
  digitalWrite(mA_dir, LOW);
  digitalWrite(mB_dir, LOW);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void rotate_r() {
  stop_m();
  digitalWrite(mA_dir, HIGH);
  digitalWrite(mB_dir, HIGH);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void stop_m() {
  digitalWrite(mA_brake, HIGH);
  digitalWrite(mB_brake, HIGH);
}

void rotation(float angle, bool clockwise) {
  float yaw;
  if (clockwise) {
    while (yaw <= angle) {
      yaw = get_gyro_angle();
    }
  } else {
    while (yaw >= angle) {
      yaw = get_gyro_angle();
    }
  }
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
