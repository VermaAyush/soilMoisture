#include <WiFi.h>
#include <EEPROM.h>
//#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "index.h"
#include <WebServer.h> 
//adafruit requirement
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "electrogoal"
#define AIO_KEY         "aio_EBff52G3qfUh2Lf6SPKFSmIXadNf"
#define FORMAT_SPIFFS_IF_FAILED true
#define redPin  19
#define greenPin 18
#define bluePin 5
#define pumpPin 21
#define adcPin 34
bool redBlink = false;
bool greenBlink = false;
bool blueBlink = false;
byte thMoist;
byte curMoist;
String JSONtxt;
unsigned long lastTime1 = 0;
unsigned long lastTime2 = 0;
char storedSSID[10];
char storedPASS[10];
unsigned long lastTime = 0;
volatile bool Mode = false;
int addr = 100;
long lastRed = 0;
long lastBlue = 0;
long lastGreen = 0;
bool pumpStat = false;
WebSocketsServer webSocket = WebSocketsServer(81);    //websocket connection
WebServer server(80);
char* ssid = "electroGoal"; //not used
char* password = "123456789";
char* mySsid = "electroGoal";

IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,0);
IPAddress netmask(255,255,255,0);

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisturevalue");
Adafruit_MQTT_Publish pump = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pump");
Adafruit_MQTT_Subscribe threshold = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/threshold");

bool redStat = false;
bool blueStat = false;
bool greenStat = false;
long redDelay = 500;
long greenDelay = 500;
long blueDelay = 500;
void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch(event) { 
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.printf("[WiFi] %d, Connected\n", event);
      Mode = 0;
    break;
    
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.printf("[WiFi]Disconnected router - Status");      
      Mode = 1;
    break;
    
     case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      Serial.printf("[WiFi] %d, AuthMode Change\n", event);
    break;
    
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.printf("[WiFi] %d, Got IP\n", event);
    break;
       
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.printf("[AP] %d, Client Connected\n", event);
      greenBlink = true;
      greenDelay = 100;
      digitalWrite(greenPin,HIGH);
      Mode = 1;
    break;
    
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.printf("[AP] %d, Client Disconnected\n", event);
      if(WiFi.getMode()==2){
        greenBlink = true;
      }
      Mode = 1;
    break;
    
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
//      Serial.printf("[AP] %d, Probe Request Recieved\n", event);
    break;
  }
}


    

void setup() {
    pinMode(redPin,OUTPUT);
    pinMode(greenPin,OUTPUT);
    pinMode(bluePin,OUTPUT);
    digitalWrite(redPin,HIGH);
    digitalWrite(greenPin,HIGH);
    digitalWrite(bluePin,HIGH);
    pinMode(pumpPin,OUTPUT);
    pinMode(adcPin, INPUT);
    EEPROM.begin(512);
    Serial.begin(115200);
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    wifiConnect();
    WiFi.onEvent(WiFiEvent);
    server.on("/",handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("HTTP server started");
    thMoist =  EEPROM.read(addr);
    Serial.println(thMoist, DEC);

     if(Mode == 0 ){
     mqtt.subscribe(&threshold);
     }
   blueBlink = false;
  Serial.println(storedSSID);
  Serial.println(storedPASS);
 
 }

void loop() {
  if(greenBlink && (millis()-lastGreen>=redDelay)){
    greenStat = !greenStat;
    digitalWrite(greenPin,greenStat);
    lastGreen = millis();  
  }

  if(redBlink && (millis()-lastRed>=greenDelay)){
    redStat = !redStat;
    digitalWrite(redPin,!redStat);
    lastRed = millis();  
  }
  if(blueBlink && (millis()-lastBlue>=blueDelay)){
    blueStat = !blueStat;
    digitalWrite(bluePin,!blueStat);
    lastBlue = millis();  
  }
  // put your main code here, to run repeatedly:
  server.handleClient();
  webSocket.loop();
  curMoist =   int(map(analogRead(adcPin), 1000, 4095, 100, 1));
if (thMoist < curMoist) {
    pumpStat = true;
  }
  else {
    pumpStat = false;
  }
   digitalWrite(pumpPin, !pumpStat);
 if(Mode ==1)
 {
  if(millis()-lastTime >= 500){
  
  String moist = String(curMoist);
  
  Serial.println(WiFi.getMode());
 // digitalWrite(pumpPin, !pumpStat);
  JSONtxt = "{\"POT\":\"" + moist + "\",\"THRESHOLD\":\"" + thMoist + "\",\"PUMP\":\"" + pumpStat + "\"}";
  webSocket.broadcastTXT(JSONtxt);
  lastTime = millis();
  }
}
    if(Mode == 0 ){
                MQTT_connect();
          
            // this is our 'wait for incoming subscription packets' busy subloop
            // try to spend your time here
          
            Adafruit_MQTT_Subscribe *subscription;
            while ((subscription = mqtt.readSubscription(5000))) {
              if (subscription == &threshold) {
                byte data = atoi((char *)threshold.lastread);
                Serial.print(F("recive data threshold: "));
                Serial.println(data,DEC);
      EEPROM.write(addr,data);
      EEPROM.commit();
      delay(10);
      thMoist =  EEPROM.read(addr);
              }
            }
          if(millis()-lastTime1>=2020){
            if(!pump.publish(!pumpStat)) {
             // digitalWrite(redPin,!digitalRead(redPin));
             
              redBlink = true;
              redDelay = 100;
              digitalWrite(bluePin,HIGH);
              Serial.println("send pump status: Fucess");
              //digitalWrite(red,LOW);
              }else{ 
              digitalWrite(bluePin,HIGH);
              redBlink = true;
              redDelay = 500;
              Serial.print("send pump status Sucess: ");
              Serial.println(pumpStat);
              //digitalWrite(bluePin,LOW);
            }
            lastTime1 = millis();
          }
          if(millis()-lastTime2>=4010){
          if (! photocell.publish(int(curMoist))) {
              redBlink = true;
              digitalWrite(bluePin,HIGH);
              redDelay = 100;
             // digitalWrite(redPin,!digitalRead(redPin));
              Serial.println(F("Send Moisture value: Failed"));
           // digitalWrite(red,LOW);
            } else {
              digitalWrite(bluePin,HIGH);              
              redBlink = true;
              redDelay = 500;
              digitalWrite(redPin,LOW);
              Serial.print(F("send Moisture value sucess : "));
              Serial.println(curMoist);
             // digitalWrite(bluePin,LOW);
            }
            lastTime2 = millis();
          }
  }
}


void wifiConnect()
{
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();          
  delay(1000);
  if(SPIFFS.exists("/config.json")){
    const char *_ssid = "", *_pass = "";
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();
      DynamicJsonDocument jsonBuffer(1024);
      DeserializationError error = deserializeJson(jsonBuffer, buf.get());
      if(error) 
      {
        return   ;
      } 
      else 
      {
        _ssid = jsonBuffer["ssid"];
        _pass = jsonBuffer["password"]; 
        strcpy(storedSSID,_ssid);
        strcpy(storedPASS,_pass); 
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid, _pass);
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED) 
        {
          delay(500);
          Serial.print(".");
          digitalWrite(redPin,LOW);
          digitalWrite(greenPin,LOW);
          digitalWrite(bluePin,!digitalRead(bluePin));
          if ((unsigned long)(millis() - startTime) >= 8000) break;
        }
      }
    }
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Mode = 0;
     digitalWrite(bluePin,HIGH);
  } else 
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, netmask);
    WiFi.softAP(mySsid, password); 
    digitalWrite(redPin,LOW);
    digitalWrite(bluePin,LOW);
    greenBlink = true;
    blueBlink = false;
    digitalWrite(greenPin,HIGH);      
    Mode = 1;
  }
  Serial.println("");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        greenBlink = false;
        digitalWrite(greenPin,HIGH);
        // send message to client
        //webSocket.sendTXT(num, "Connected");
      }
    break; 
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
     // hexdump(payload, length);
      digitalWrite(redPin,HIGH);
      digitalWrite(greenPin,HIGH);
      digitalWrite(bluePin,HIGH);
      
      Serial.print("[");
      Serial.print(*payload, DEC);
      Serial.println("]");
      EEPROM.write(addr, *payload);
      EEPROM.commit();
      delay(10);
      thMoist =  EEPROM.read(addr);
      digitalWrite(redPin,LOW);
//      digitalWrite(greenPin,LOW);
      digitalWrite(bluePin,LOW);
      
      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String data = String((char*)( payload));
      DynamicJsonDocument jBuffer(1024);
      DeserializationError error = deserializeJson(jBuffer, data);
      File configFile = SPIFFS.open("/config.json", "w");
      serializeJson(jBuffer, configFile);
      configFile.close();
      delay(500);
      
      wifiConnect();
      ESP.restart();
      break;
      }
}
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  
 
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(3000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
        digitalWrite(redPin,HIGH);
         Serial.println("[MQTT Broker not reachable]");
       }
  }
  
  digitalWrite(redPin,LOW);

  Serial.println("MQTT Connected!");
}

void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}
