#include <WiFi.h>
#include <WebSocketsServer.h>

#define RX 16
#define TX 17
#define MAX_CLIENTS 5

const char* ssid = "test_ssid";
const char* pass = "password";
const int port = 9000;
const int ws_port = 9001;

typedef struct {
  bool set;
  WiFiClient *client;
  bool ws_sock;
  uint8_t id;
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

WebSocketsServer ws = WebSocketsServer(ws_port);

Subscriber subscribers[MAX_CLIENTS];

bool x1 = false, x2 = false;

void notify(char d, String str) {
  str += '_';
  bool sen = false;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (subscribers[i].set) {
      Serial.println("swt");
      switch (d) {
        case 'D':
          if (subscribers[i].directions) {
            sen = true;
          }
          break;
        case 'R':
          if (subscribers[i].rotation) {
            sen = true;
          }
          break;
        case 'U':
          if (subscribers[i].ultrasonic) {
            sen = true;
            Serial.println("ssafgd");
          }
          break;
        case 'S':
          if (subscribers[i].servo) {
            sen = true;
          }
          break;
      }
      if (sen) {
        Serial.println("sen");
        if (subscribers[i].ws_sock) {
          Serial.println("sock yes");
          ws.sendTXT(subscribers[i].id, str);
        } else {
          Serial.println("sock no");
          subscribers[i].client->println(str);
        }
      }
    }
  }
}

String converter(uint8_t *str){
    return String((char *)str);
}

void wsEvent(uint8_t id, WStype_t type, uint8_t * dat, size_t length) {
  String data = converter(dat);
  Serial.println("Event o");
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("deleted client");
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (subscribers[i].set) {
          subscribers[i].set = false;
          subscribers[i].subscribed = false;
          break;
        }
      }
      break;
    case WStype_CONNECTED:
      Serial.println("New Client");
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!subscribers[i].set) {
          subscribers[i].set = true;
          subscribers[i].id = id;
          subscribers[i].ws_sock = true;
          subscribers[i].subscribed = false;
          subscribers[i].directions = false;
          subscribers[i].rotation = false;
          subscribers[i].ultrasonic = false;
          subscribers[i].servo = false;
          break;
        }
      }
      break;
    case WStype_TEXT:
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (subscribers[i].ws_sock) {
          if (subscribers[i].subscribed) {
            Serial.println(data);
            String dd = data.substring(0,3);
            if (dd == "FWD" || dd == "BWD" ||
              dd == "LFT" || dd == "RGT" || dd == "STP") {
              notify('D', dd);
            } else if (dd == "ROL" || dd == "ROR") {
              notify('R', dd);
            } else if (dd == "ULT") {
              Serial.println("ddadaf");
              notify('U', dd);
            } else if (data.charAt(0) == 'S' &&
              data.charAt(1) == 'R' && data.charAt(2) == 'V') {
              notify('S', data);
            }
          } else {
            if (data.charAt(0) == 'S' &&
              data.charAt(1) == 'U' && data.charAt(2) == 'B') {
              Serial.println("new subscription");
              subscribers[i].subscribed = true;
              char k = 3;
              Serial.println(data.charAt(k));
              while (data.charAt(k) != '_') {
                switch (data.charAt(k)) {
                  case 'D':
                    Serial.print("Set up: ");
                    Serial.println(data.charAt(k));
                    subscribers[i].directions = true;
                    break;
                  case 'R':
                    Serial.print("Set up: ");
                    Serial.println(data.charAt(k));
                    subscribers[i].rotation = true;
                    break;
                  case 'U':
                    Serial.print("Set up: ");
                    Serial.println(data.charAt(k));
                    subscribers[i].ultrasonic = true;
                    break;
                  case 'S':
                    Serial.print("Set up: ");
                    Serial.println(data.charAt(k));
                    subscribers[i].servo = true;
                    break;
                }
                k++;
                Serial.println(data.charAt(k));
              }
              ws.sendTXT(id, "Subscribed_");
            }
          }
        }
      }
      break;
    default:
      Serial.println("default");
      break;
  }
}

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

  ws.begin();
  ws.onEvent(wsEvent);
}

void loop() {
  ws.loop();
  
  WiFiClient c = s.available();

  
  if (c) {
    Serial.println("New Client");
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (!subscribers[i].set) {
          subscribers[i].set = true;
          subscribers[i].ws_sock = false;
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
    if (subscribers[i].set && !subscribers[i].ws_sock) {
      if (!subscribers[i].client->connected()) {
        subscribers[i].client->stop();
        delete subscribers[i].client;
        subscribers[i].client = NULL;
        subscribers[i].set = false;
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
              //check if they're registed for the action
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
