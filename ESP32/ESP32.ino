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
  
  /*WiFiClient c = s.available();


  
  //start and stop, http vs others
  bool y = true;
  bool http = false;
  bool com = false;
  String str = "";
  while (c.connected()) {
    if (y) {y=false;Serial.println("start");}
    char x = char(c.read());
    Serial.print(x);
    if (com) {
      str += x;
      if (x == '_') {
        if (http) {
          c.println("HTTP/1.1 200 OK");
          c.println("Content-type:text/html");
          c.println("Access-Control-Allow-Origin: *");
          //c.println("Connection: close");
          c.println();
        }
        //c.stop();
        str = "";
        com = false; 
        Serial.println("shold stop");
        break;
      }
    } else if (x1 && x2 && x=='_') {
      com = true;
      x1 = false;
      x2 = false;
      http = false;
    } else if (x1 && (x=='S' || x=='H')) {
      x2 = true;
      if (x=='H') {
        http = true;
      }
    } else if (x=='_') {
      x1 = true;
    } else {
      x1 = false;
      x2 = false;
    }
  }*/



  
  /*if (c) {
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
  }*/
}

void wsEvent(uint8_t id, WStype_t type, uint8_t * data, size_t length) {
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
          break;
        }
      }
      break;
    case WStype_TEXT:
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (subscribers[i].ws_sock) {
          if (subscribers[i].subscribed) {
            Serial.println(data);
            if (data == "FWD" || data == "BWD" ||
              data == "LFT" || data == "RGT" || data == "STP") {
              notify('D', data);
            } else if (data == "ROL" || data == "ROR") {
              notify('R', data);
            } else if (data == "ULT") {
              notify('U', data);
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
      break;
  }
}

void notify(char d, String str) {
  str += '_';
  bool sen = false;
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (subscribers[i].set) {
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
          }
          break;
        case 'S':
          if (subscribers[i].servo) {
            sen = true;
          }
          break;
      }
      if (sen) {
        if (subscribers[i].ws_sock) {
          ws.sendTXT(subscribers[i].id, "str);
        } else {
          subscribers[i].client->println(str);
        }
      }
    }
  }
}
