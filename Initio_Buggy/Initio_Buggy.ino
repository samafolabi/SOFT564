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

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    
  }
  
  Serial.println("Initialising...");
  
  //pinMode(mA_dir, OUTPUT);
  //pinMode(mA_brake, OUTPUT);
  //pinMode(mB_dir, OUTPUT);
  //pinMode(mB_brake, OUTPUT);

  //esp32.begin(9600);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
  calculate_IMU_error();
  Serial.print("IMU Error: ");
  Serial.println(GyroErrorZ);

  //remote.enableIRIn();

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  long ultra_duration;
  int ultra_distance;

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
  delay(1500);

  Serial.println("sending");
  esp32.print("S");
  bool x = true;
  char c = 10;
  while (true) {
    if (esp32.available()) {
      char g = char(esp32.read());
      if (g == '.') {
        c--;
        if (c == 0) {Serial.println("Wifi connection failed");c=10;}
      } else if (g == 'C') {
        while (x) {
          if (esp32.available()) {
            char d = char(esp32.read());
            if (d == 'F') {
              x = false;
            } else if (d == 'S') {
              int j = 1;
              while (true) {
                esp32.print("Message ");
                esp32.println(j++);
                delay(5000);
              }
            }
          }
        }
      }
    }
  }
  */get_gyro_angle();
  Serial.println(yaw);
  delay(1000);

  /*if (remote.decode(&results)) {     
    int value = results;
    Serial.println(" ");     
    Serial.print("Code: ");     
    Serial.println(value);    
    Serial.println(" ");     
    irrecv.resume();
  }

  for(servo_pos = 0; servo_pos < 180; servo_pos += 1) {
    servo.write(servo_pos);
    delay(15);
  }
  
  for(servo_pos = 180; servo_pos >= 1; servo_pos -= 1) {
    servo.write(servo_pos);
    delay(15);
  }

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  ultra_duration = pulseIn(echoPin, HIGH);
  ultra_distance = ultra_duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(ultra_distance);
  Serial.println(" cm");*/
}

void forward() {
  digitalWrite(mA_dir, HIGH);
  digitalWrite(mB_dir, LOW);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void backward() {
  digitalWrite(mA_dir, LOW);
  digitalWrite(mB_dir, HIGH);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void rotate_l() {
  digitalWrite(mA_dir, LOW);
  digitalWrite(mB_dir, LOW);
  digitalWrite(mA_brake, LOW);
  digitalWrite(mB_brake, LOW);
  digitalWrite(mA_speed, m_speed);
  digitalWrite(mB_speed, m_speed);
}

void rotate_r() {
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
