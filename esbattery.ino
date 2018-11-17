#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "ADS1115.h"
const int node = 2;
const char* ssid = "";
const char* password = "";
String localip = "0.0.0.0";
ESP8266WebServer server(80);
IPAddress addr (192,168,1,200 + node);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(0,0,0,0);

ADS1115 ads;

const int led = D4;

void handleRoot() {
  int adc0;
  String buf = "{";
  digitalWrite(led, 1);
  ads.getAddr_ADS1115(ADS1115_GND_ADDRESS);
  Wire.beginTransmission(ADS1115_GND_ADDRESS);
  int error = Wire.endTransmission();
  if (error == 0)
  {
    int ad0 = ads.Measure_SingleEnded(0);
    int ad3 = ads.Measure_SingleEnded(3);
    if (ad0 & 0x8000 ) ad0 = 0;
    if (ad3 & 0x8000 ) ad3 = 0;
    buf += " \"ch0\": ";
    buf += ad0 * 1.875 /1000 ;
    buf += ",";
    buf += " \"vcc\": ";
    buf += ad3 * 1.875 / 10000;
    buf += ",";
    //
    buf += " \"status\": \"good\", ";    
  } else {
    Serial.println ("ERROR");
    buf += "\"status\": \"bad\", ";
  }
  buf += " \"node\": " + String(node) +  ", ";
  buf += " \"ip\": \"" + localip + "\" ";
  buf += "}\n";
  server.send(200, "text/plain", buf);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  Serial.begin(115200);
  ads.begin();
  ads.getAddr_ADS1115(ADS1115_GND_ADDRESS); 
  ads.setGain(GAIN_TWOTHIRDS);  
  ads.setMode(MODE_CONTIN);      
  ads.setRate(RATE_475);
  int adc0;
  ads.getAddr_ADS1115(ADS1115_GND_ADDRESS);
  WiFi.mode (WIFI_STA);
  WiFi.config(addr, gateway, subnet);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  Serial.println ("start wifi");
  WiFi.begin(ssid, password);
  Serial.println("wait");
  while (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print("X");
    if (digitalRead (led) > 0) {
      digitalWrite (led , LOW);
    } else {
      digitalWrite (led, HIGH);
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  localip = WiFi.localIP().toString().c_str();
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("Started");
}

void loop(void) {
  server.handleClient();
}
