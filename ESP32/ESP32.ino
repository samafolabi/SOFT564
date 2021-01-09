#include <WiFi.h>

#define RX 16
#define TX 17
#define MAX_CLIENTS 5

const char* ssid = "test_ssid";
const char* pass = "password";
const int port = 9000;

typedef struct {
  WiFiClient *client;
  bool subscribed;
  bool directions;
  bool rotation;
  bool ultrasonic;
  bool servo;
} Subscriber;

WiFiServer s(port);
IPAddress IP(192, 168, 43, 21);
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 0, 0);

Subscriber subscribers[MAX_CLIENTS];

void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);

  WiFi.config(IP, gateway, subnet);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.print("WiFi connected with IP at ");
  Serial.println(WiFi.localIP());
  s.begin();
}

void loop() {
  WiFiClient c = s.available();
  if (c) {
    Serial.println("New Client");
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (subscribers[i].client == NULL) {
        subscribers[i].client = new WiFiClient(c);
        subscribers[i].subscribed = false;
        subscribers[i].directions = false;
        subscribers[i].rotation = false;
        subscribers[i].ultrasonic = false;
        subscribers[i].servo = false;
        break;
      }
    }
  }

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (subscribers[i].client != NULL) {
      if (!subscribers[i].client->connected()) {
        subscribers[i].client->stop();
        delete subscribers[i].client;
        subscribers[i].client = NULL;
        subscribers[i].subscribed = false;
        subscribers[i].directions = false;
        subscribers[i].rotation = false;
        subscribers[i].ultrasonic = false;
        subscribers[i].servo = false;
        Serial.println("deleted client");
      } else if (subscribers[i].client->available()) {
        String line = "";
        while (subscribers[i].client->connected()) {
          char h = subscribers[i].client->read();
          line += h;
          if (line.length() == 3) {
            if (subscribers[i].subscribed) {
              Serial.println(line);
              if (line == "FWD" || line == "BWD" || line == "LFT" || line == "RGT" || line == "STP") {
                notify('D', line);
              } else if (line == "ROL" || line == "ROR") {
                notify('R', line);
              } else if (line == "ULT") {
                notify('U', line);
              } else if (line == "SRV") {
                notify('S', line);
              }
            } else {
              if (line == "SUB") {
                Serial.println("new subscription");
                subscribers[i].subscribed = true;
                char k = subscribers[i].client->read();
                Serial.println(k);
                while (k != '_') {
                  switch (k) {
                    case 'D':
                      subscribers[i].directions = true;
                      break;
                    case 'R':
                      subscribers[i].rotation = true;
                      break;
                    case 'U':
                      subscribers[i].ultrasonic = true;
                      break;
                    case 'S':
                      subscribers[i].servo = true;
                      break;
                  }
                  k = subscribers[i].client->read();
                  Serial.println(k);
                }
                subscribers[i].client->println("Subscribed_");
              }
            }
            break;
          }
        }
      }
    }
  }
}

void notify(char d, String str) {
  str += '_';
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (subscribers[i].client != NULL && subscribers[i].client->connected()) {
      switch (d) {
        case 'D':
          if (subscribers[i].directions) {
            subscribers[i].client->println(str);
          }
          break;
        case 'R':
          if (subscribers[i].rotation) {
            subscribers[i].client->println(str);
          }
          break;
        case 'U':
          if (subscribers[i].ultrasonic) {
            subscribers[i].client->println(str);
          }
          break;
        case 'S':
          if (subscribers[i].servo) {
            subscribers[i].client->println(str);
          }
          break;
      }
      Serial.println("notified");
    }
  }
}
