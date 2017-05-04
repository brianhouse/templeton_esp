#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

ADC_MODE(ADC_VCC);

WiFiUDP Udp;

const String idString = "1";
const char* ssid      = "3V8VC";
const char* password  = "7YYGM8V3R65V52FJ";
const char* host      = "192.168.1.8";
const int port        = 23232; // both send and receive

int lit = 0;
int itr = 0;

void setup() {
  Serial.begin(115200);
  delay(1);
  pinMode(2, OUTPUT);  
  pinMode(14, INPUT_PULLUP);
  connectToWifi();
  Udp.begin(port);
}

void loop() {
  Udp.beginPacket(host, port);
  int tilt = digitalRead(14);
  String dataString = idString + "," + String(WiFi.RSSI()) + "," + String(ESP.getVcc()) + "," + String(tilt);
  Serial.println(dataString);
  char dataBuf[dataString.length()+1];
  dataString.toCharArray(dataBuf, dataString.length()+1);
  Udp.write(dataBuf);
  Udp.endPacket();      
  itr += 1;
  if (itr == 100) {
    if (lit == 1) {
      digitalWrite(2, HIGH);      
      lit = 0;
    } else {
      digitalWrite(2, LOW);
      lit = 1;
    }
    itr = 0;
  }
  delay(10);
}

void connectToWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println();
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    int tries = 0;
    while (tries < 20 && WiFi.status() != WL_CONNECTED) {
      digitalWrite(2, HIGH);
      delay(250);
      digitalWrite(2, LOW);
      delay(250);      
      Serial.print(".");
      tries++;
    }
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      tries = 0;
      while (tries < 20) {
        digitalWrite(2, HIGH);
        delay(250);
        digitalWrite(2, LOW);
        delay(250);
        tries++;
      }
    }
  }
  Serial.println();
  Serial.println("--> connected to wifi");
  digitalWrite(0, HIGH);
  printWifiStatus();
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.println(rssi);
}

