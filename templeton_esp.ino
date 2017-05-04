#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

ADC_MODE(ADC_VCC);

WiFiUDP Udp;
Adafruit_MMA8451 mma = Adafruit_MMA8451();

const char* ssid      = "3V8VC";
const char* password  = "7YYGM8V3R65V52FJ";
const char* host      = "192.168.1.8";
const int port        = 23232; // both send and receive

unsigned long ms;
unsigned long seconds;
String idString;

void setup() {
  Serial.begin(115200);
  delay(1);
  int id = ESP.getFlashChipId();
  idString = String(id);  
  Serial.print("ID: ");
  Serial.println(idString);  
  Wire.begin(12, 14);    // SDA, SCL
  pinMode(2, OUTPUT);  
  pinMode(14, INPUT_PULLUP);
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);  
  connectToWifi();
  Udp.begin(port);
}

void loop() {
  ms = millis();
  seconds = ms / 1000;
  if (seconds % 2 == 0) {
    digitalWrite(2, LOW);  
  } else {
    digitalWrite(2, HIGH);  
  }
  mma.read();
  sensors_event_t event; 
  mma.getEvent(&event);  
  Udp.beginPacket(host, port);
  int tilt = digitalRead(14);
  String dataString = idString + "," + String(WiFi.RSSI()) + "," + String(ESP.getVcc()) + "," + ms + "," + String(tilt) + "," + String(event.acceleration.x, 8) + "," + String(event.acceleration.y, 8) + "," + String(event.acceleration.z, 8);  
  Serial.println(dataString);
  char dataBuf[dataString.length()+1];
  dataString.toCharArray(dataBuf, dataString.length()+1);
  Udp.write(dataBuf);
  Udp.endPacket();      
  delay(10);
}

void connectToWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println();
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      digitalWrite(2, HIGH);
      delay(250);
      digitalWrite(2, LOW);
      delay(250);      
      Serial.print(".");
    }
  }
  Serial.println();
  Serial.println("--> connected to wifi");
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

