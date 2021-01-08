#include <WiFi.h>

#define RX 16
#define TX 17

const char* ssid = "test_ssid";
const char* pass = "password";
const int port = 9000;
const char* host = "192.168.43.210";

WiFiClient client;

void setup() {
  Serial2.begin(9600, SERIAL_8N1, RX, TX);
  WiFi.begin(ssid, pass);
  pinMode(2, OUTPUT);
}

void loop() {
  while (!Serial2.available()) {
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
  }
  if (char(Serial2.read()) == 'S') {
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial2.println(".");
    }
    Serial2.print("C");
    if (!client.connect(host, port)) {
      Serial2.println("F");
    } else {
      Serial2.println("S");
      client.print("Hello from ESP32!");

      while (true) {
        while (Serial2.available()) {
          client.print(Serial2.readString());
        }
      }
    }
  }

  
}
