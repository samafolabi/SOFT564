#include <Wifi.h>

#define RX 16
#define TX 17

const char* ssid = "test_ssid";
const char* pass = "password";
const char* port = 9000;
const char* host = "192.168.1.1";

WiFiClient client;

void setup() {
  Serial2.begin(115200, SERIAL_8N1, RX, TX);
  Wifi.begin(ssid, pass);
}

void loop() {
  while (!Serial2.available());
  if (char(Serial2.read()) == 'S') {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial2.println(".");
    }
   
    Serial2.print("C");
    if (!client.connect(host, port)) {
      Serial2.println("F");
    } else {
      Serial2.println("S");
      client.print("Hello from ESP32!");
    }
  }

  while (true) {
    while (Serial2.available()) {
      client.print(Serial2.readString());
    }
  }
}
