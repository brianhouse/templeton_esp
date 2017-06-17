#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
extern "C" {
  #include "user_interface.h"
}

ADC_MODE(ADC_VCC);

WiFiUDP Udp;
Adafruit_MMA8451 mma = Adafruit_MMA8451();

const char* ssid      = "3V8VC";
const char* password  = "7YYGM8V3R65V52FJ";
const char* host      = "192.168.1.5";
//const char* ssid     = "GL-MT300N-5cb";
//const char* password = "goodlife";
//const char* host     = "192.168.8.235";
const int port        = 23232; // both send and receive

unsigned long ms;
unsigned long seconds;
unsigned long last_seconds;
String idString;

uint32_t mem;

char data[700][40]; // 1000 rows of 39(+1) chars at 25 hz is 40 seconds worth of data that can be stored before transmitting.
int dataIndex = 0;


void setup() {
  Serial.begin(115200);
  delay(1);
  int id = ESP.getChipId();
  idString = String(id);  
  pinMode(2, OUTPUT);  

//  // allow forced modem sleeping
//  wifi_set_sleep_type(MODEM_SLEEP_T);
//  Serial.println(MODEM_SLEEP_T);
//  delay(1);

  // accel
  Wire.begin(12, 14);    // SDA, SCL
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);  

//  // tilt
//  pinMode(14, INPUT_PULLUP);
  
  connectToWifi();
  Udp.begin(port);

  last_seconds = 0;
}

void loop() {
  ms = millis();
  seconds = ms / 1000;
  if (seconds % 2 == 0) {
    digitalWrite(2, LOW);  
  } else {
    digitalWrite(2, HIGH);  
  }

//  // simple tilt sensor
//  int tilt = digitalRead(14);
//  String dataString = idString + "," + String(WiFi.RSSI()) + "," + String(ESP.getVcc()) + "," + ms + "," + String(tilt) + ",0,0";  

  // accelerometer
  mma.read();
  sensors_event_t event; 
  mma.getEvent(&event);  
  String mag = String(sqrt((event.acceleration.x * event.acceleration.x) + (event.acceleration.y * event.acceleration.y) + (event.acceleration.z * event.acceleration.z)) - 9.8);
  
  String dataString = idString + "," + String(WiFi.RSSI()) + "," + String(ESP.getVcc()) + "," + ms + "," + mag + ",";
  dataString.toCharArray(data[dataIndex], 40);  // room to grow
  dataIndex = (dataIndex + 1) % sizeof(data);


  if (seconds != last_seconds) {
    Serial.print("T: ");    
    Serial.println(seconds);
    mem = system_get_free_heap_size();
    Serial.print("Free memory: ");
    Serial.println(mem);
    Serial.print("data: ");        
    Serial.println(dataString.length());
  }

  sendData(dataString);

  last_seconds = seconds;
  delay(10);
}

void sendData(String dataString) {
  if (WiFi.status() != WL_CONNECTED) {  // dont use this if in forced sleep
    connectToWifi();
  }  
  Udp.beginPacket(host, port);  
  char dataBuf[dataString.length()+1];
  dataString.toCharArray(dataBuf, dataString.length()+1);
  Udp.write(dataBuf);
  Udp.endPacket();      
}

void connectToWifi() {
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
  Serial.println();
  Serial.println("--> connected to wifi");
}


