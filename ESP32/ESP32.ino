#include <WiFi.h>
#include <WebSocketsServer.h>
#include <IRremote.h>

//Arduino Serial pins
#define RX 16
#define TX 17

#define MAX_CLIENTS 5
#define ir 15 //IR receiver

//WiFi and Server details
const char* ssid = "test_ssid";
const char* pass = "password";
const int port = 9000;
const int ws_port = 9001;

typedef struct {
  bool set; //initialised
  WiFiClient *client; //client object for socket
  bool ws_sock; //true - WebSocket, false - socket
  uint8_t id; //client number for WebSocket
  bool subscribed; //client subscription status
  
  //Component Configurations
  bool directions;
  bool rotation;
  bool ultrasonic;
  bool servo;
} Subscriber;

//Server and IPAddress Setup
WiFiServer s(port);
IPAddress IP(192, 168, 43, 21);
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 0, 0);
WebSocketsServer ws = WebSocketsServer(ws_port);

IRrecv remote(ir);

Subscriber subscribers[MAX_CLIENTS]; //Subscriber list

decode_results results; //IR receiver results
unsigned long remote_value = 0;

//Notification function to send messages about
//component changes to subscribed clients
void notify(char d, String str) {
  //Loop through clients to see if initialised
  for (int i = 0; i < MAX_CLIENTS; i++) {
    bool sen = false;
    if (subscribers[i].set) {
      //Check if subscribed to particular component
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
      //If subscribed, send via appropriate methods
      if (sen) {
        if (subscribers[i].ws_sock) {
          ws.sendTXT(subscribers[i].id, str);
        } else {
          subscribers[i].client->println(str);
        }
      }
    }
  }
}

//Change data types due to wsEvent parameters
String converter(uint8_t *str){
    return String((char *)str);
}

//Work with connection depending on type of event
//From: https://shawnhymel.com/1675/arduino-websocket-server-using-an-esp32/
void wsEvent(uint8_t id, WStype_t type, uint8_t * dat, size_t length) {
  String data = converter(dat);
  switch(type) {
    case WStype_DISCONNECTED:
      //If disconnected, reset init and subscription
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (subscribers[i].set) {
          subscribers[i].set = false;
          subscribers[i].subscribed = false;
          break;
        }
      }
      break;
      
    case WStype_CONNECTED:
      //If it is a new connection, set the id, init, socket type,
      //subscription, and configurations
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
      //If there is data, decode the first three characters
      //and perform the right action
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (subscribers[i].ws_sock) {
          if (subscribers[i].subscribed) {
            
            String dd = data.substring(0,3);
            
            if (dd == "FWD" || dd == "BWD" ||
              dd == "LFT" || dd == "RGT" || dd == "STP") {
              notify('D', dd);
              Serial2.print('_');
              Serial2.print(dd);
            } else if (dd == "ROL" || dd == "ROR") {
              Serial2.print('_');
              Serial2.print(dd);
              notify('R', dd);
            } else if (dd == "ULT") {
              Serial2.print('_');
              Serial2.print(dd);
              
              //If it is an ultrasonic command, send the command
              //to the Arduino and wait for its response to send back
              
              String us = "_";
              while (true) {
                if (Serial2.available()) {
                  char u = char(Serial2.read());
                  if (u == '_') {
                    dd += us;
                    notify('U', dd);
                    break;
                  } else {
                    us += u;
                  }
                }
              }
            } else if (data.charAt(0) == 'S' &&
              data.charAt(1) == 'R' && data.charAt(2) == 'V') {
              Serial2.print('_');
              notify('S', data);
              Serial2.print(data);
            }
          } else {
            if (data.charAt(0) == 'S' &&
              data.charAt(1) == 'U' && data.charAt(2) == 'B') {
              //If the client is not subscribed and wants to subscribe,
              //read until end character '_' and set the corresponding
              //configurations
              
              subscribers[i].subscribed = true;
              char k = 3;
              while (data.charAt(k) != '_') {
                switch (data.charAt(k)) {
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
              }
              ws.sendTXT(id, "Subscribed");
            }
          }
        }
      }
      break;
    default:
      break;
  }
}

void setup() {
  //Set pin mode of onboard LED
  pinMode(2, OUTPUT);

  //Initialise IR receiver
  remote.enableIRIn();
  remote.blink13(true);

  //Set static IP, connect to WiFi, then set up the WebSocket events
  WiFi.config(IP, gateway, subnet);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  s.begin();

  ws.begin();
  ws.onEvent(wsEvent);
}

void loop() {
  //If there is no data from the Arduino, blink the LED
  while (!Serial2.available()) {
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
  }
  
  if (char(Serial2.read()) == 'S') {
    while (true) {
      
      ws.loop(); //Check for WebSocket events
  
      WiFiClient c = s.available(); //Check for new socket
                                    //Connections
    
      //If it is a new connection, set the client, init, socket type,
      //subscription, and configurations
      if (c) {
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
            //If disconnected, reset init and subscription,
            //stop the client, and delete the client object
            subscribers[i].client->stop();
            delete subscribers[i].client;
            subscribers[i].client = NULL;
            subscribers[i].set = false;
            subscribers[i].subscribed = false;
            subscribers[i].directions = false;
            subscribers[i].rotation = false;
            subscribers[i].ultrasonic = false;
            subscribers[i].servo = false;
          } else if (subscribers[i].client->available()) {
            String line = "";
            while (subscribers[i].client->connected()) {
              //If there is data, decode the first three characters
              //and perform the right action
              char h = subscribers[i].client->read();
              line += h;
              if (line.length() == 3) {
                if (subscribers[i].subscribed) {
                  //check if they're registed for the action
                  if (line == "FWD" || line == "BWD" || line == "LFT" || line == "RGT" || line == "STP") {
                    Serial2.print('_');
                    notify('D', line);
                    Serial2.print(line);
                  } else if (line == "ROL" || line == "ROR") {
                    Serial2.print('_');
                    notify('R', line);
                    Serial2.print(line);
                  } else if (line == "ULT") {
                    Serial2.print('_');
                    Serial2.print(line);
                    String us = "_";
                    //If it is an ultrasonic command, send the command
                    //to the Arduino and wait for its response to send back
                    
                    while (true) {
                      if (Serial2.available()) {
                        char u = char(Serial2.read());
                        if (u == '_') {
                          line += us;
                          notify('U', line);
                          break;
                        } else {
                          us += u;
                        }
                      }
                    }
                  } else if (line == "SRV") {
                    Serial2.print('_');
                    h = subscribers[i].client->read();
                    while (h != '_') {
                      line += h;
                      h = subscribers[i].client->read();
                    }
                    notify('S', line);
                    Serial2.print(line);
                  }
                } else {
                  if (line == "SUB") {
                    //If the client is not subscribed and wants to subscribe,
                    //read until end character '_' and set the corresponding
                    //configurations
                    
                    subscribers[i].subscribed = true;
                    char k = subscribers[i].client->read();
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
                    }
                    subscribers[i].client->println("Subscribed");
                  }
                }
                break;
              }
            }
          }
        }
      }

      //If the receiver gets a new code, save it and check
      //the value. If set previously, send the stop code.
      //If not, send the right code
      if (remote.decode(&results)) {
        remote.resume();
        delay(200);
        String line = "";
        if (results.value == remote_value) {
          remote_value = 0;
          line = "STP";
        } else {
          switch (results.value) {
            case 16718055:
              line = "FWD";
              remote_value = results.value;
              break;
            case 16730805:
              line = "BWD";
              remote_value = results.value;
              break;
            case 16716015:
              line = "LFT";
              remote_value = results.value;
              break;
            case 16734885:
              line = "RGT";
              remote_value = results.value;
              break;
            default:
              break;
          }
          Serial2.print('_');
          Serial2.print(line);
        }
      }

      
    }
  }
  
}
