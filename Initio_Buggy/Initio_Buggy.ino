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
#define m_speed 255 //motor speed

#define esp32_tx 2
#define esp32_rx 4
#define servo_pin 10
#define trig 6
#define echo 7

#define MPU 0x68 //I2C address for gyroscope

float GyroErrorZ = 0, yaw = 0;
float elapsedTime, currentTime = 0, previousTime;

SoftwareSerial esp32(esp32_rx, esp32_tx);
Servo servo;

int servo_pos = 90;

//Convert servo angle (0-180) to gyroscope angle (90-(-90))
int rev_angle_map(int angle) {
  angle = angle - 90;
  angle = angle * -1;
  return angle;
}

//Turn characters into a number depending on number of digits
int con_str_int(char c1, char c2, char c3, char i) {
  int a = 0;
  if (i == 3) {
    //subtract by '0' because of ASCII numbering
    a = ((c1 - '0')*100) + ((c2 - '0')*10) + (c3 - '0');
  } else if (i == 2) {
    a = ((c1 - '0')*10) + (c2 - '0');
  } else if (i == 1) {
    a = (c1 - '0');
  }
  return a;
}

void setup() {
  //Set the pin modes of the motors and set brakes to low
  pinMode(mA_dir, OUTPUT);
  pinMode(mA_brake, OUTPUT);
  pinMode(mA_speed, OUTPUT);
  pinMode(mB_speed, OUTPUT);
  pinMode(mB_dir, OUTPUT);
  pinMode(mB_brake, OUTPUT);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);

  //Initialise serial comms to ESP32
  esp32.begin(9600);

  //Initialise Wire library and reset sensore
  //From https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  //Get average error of gyroscope to subtract
  calculate_IMU_error();

  //Set pin modes of ultrasonic sensor
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  //Attach pin to servo and set to initial position
  servo.attach(servo_pin);
  servo.write(servo_pos);
}

void loop() {
  delay(1000); //Delay to allow ESP32 to initialise
  esp32.print("S"); //Send character to ESP32 to announce readiness

  //Infinite loop that polls ESP32 for data
  //When a special character is read, start reading the command
  //into an array. Once the count reaches 3, check command
  char command[3];
  char command_count = 0;
  bool command_start = false;
  while (true) {
    if (esp32.available()) {
      //If the command check has not started and the special
      //character '_' is read, start the command check
      if (!command_start && char(esp32.read()) == '_') {
        command_start=true;
        continue;
      }
      
      if (command_start) {
        command[command_count++]=char(esp32.read());
        if (command_count == 3) {

          //Command decoding
          if (command[0] == 'F' && command[1] == 'W' && command[2] == 'D') {
            forward();
          } else if (command[0] == 'B' && command[1] == 'W' && command[2] == 'D') {
            backward();
          } else if (command[0] == 'L' && command[1] == 'F' && command[2] == 'T') {
            rotate_l();
          } else if (command[0] == 'R' && command[1] == 'G' && command[2] == 'T') {
            rotate_r();
          } else if (command[0] == 'S' && command[1] == 'T' && command[2] == 'P') {
            stop_m();
          } else if (command[0] == 'R' && command[1] == 'O' && command[2] == 'L') {
            rotation(float(rev_angle_map(0)), false);
          } else if (command[0] == 'R' && command[1] == 'O' && command[2] == 'R') {
            rotation(float(rev_angle_map(180)), true);
          } else if (command[0] == 'U' && command[1] == 'L' && command[2] == 'T') {
            //check the ultrasonic distance and send it back
            int us = ultrasonic();
            esp32.print(us);
            esp32.print('_');
          } else if (command[0] == 'S' && command[1] == 'R' && command[2] == 'V') {
            //If it is a servo command, read the next characters
            //until a '_' is reached (guaranteed to be 3 characters max)
            //Once reached, change characters to a number and
            //set the servo position
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
                  set_servo_pos(t);
                  break;
                }
              }
            }
          }

          //Reset the command
          command_count = 0;
          command[0] = 0;
          command[1] = 0;
          command[2] = 0;
          command_start = false;
        }
      }
    }
  }
}

//Motor direction functions
//Stop the motors, set the directions, then set speed
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

//Stop motor
//Set to zero and delay to prevent quick changing
void stop_m() {
  analogWrite(mA_speed, 0);
  analogWrite(mB_speed, 0);
  delay(500);
}

//Rotate the buggy either clockwise or counterclockwise
//until the yaw reaches the specified angle
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

//Read from the gyroscope and integrate it with past values
//From: https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
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

//From: https://howtomechatronics.com/tutorials/arduino/arduino-and-mpu6050-accelerometer-and-gyroscope-tutorial/
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

//Set the pulse on the TRIG pin and wait for a pulse
//on the ECHO pin, then multiply by the speed of sound
//and divide by 2 due to the round trip
//From: https://create.arduino.cc/projecthub/abdularbi17/ultrasonic-sensor-hc-sr04-with-arduino-tutorial-327ff6
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
