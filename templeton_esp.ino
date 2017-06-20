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
const char* host      = "192.168.1.18";
//const char* ssid     = "GL-MT300N-5cb";
//const char* password = "goodlife";
//const char* host     = "192.168.8.235";
const int port        = 23232; // both send and receive
const int rows        = 34; 
const int entry       = 30; // these have to multiply to < 1024 for UDP

unsigned long ms;
unsigned long seconds;
unsigned long last_seconds = 0;
String idString;

uint32_t mem;

char data[(rows * entry) + 1]; // 1000 rows of 40 chars at 25 hz is 40 seconds worth of data that can be stored before transmitting.
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

  // initialize buffer and store end byte
  for (int i=0; i < (rows * entry); i++) {
    data[i] = 'x';
  }
  data[(rows * entry)] = '\0';

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
  float bat = lround(((ESP.getVcc() - 2724.0) / (3622.0 - 2724.0)) * 100); // constants for adafruit battery, 3.0v-4.2v
  
  String dataString = idString + "," + String(WiFi.RSSI() * -1) + "," + String(bat) + "," + ms + "," + mag + ",";
  char tempBuf[dataString.length() + 1];
  dataString.toCharArray(tempBuf, dataString.length() + 1);
  for (int i=0; i<sizeof(tempBuf)-1; i++) {
      data[(dataIndex * entry) + i] = tempBuf[i];
  }

//  if (seconds != last_seconds) {
//    mem = system_get_free_heap_size();
//    Serial.print("Free memory: ");
//    Serial.println(mem);
//  }

  if (dataIndex == rows - 1) {
    Serial.print("Transmitting...");
    sendData();
    Serial.println(" done.");
  }
  dataIndex = (dataIndex + 1) % rows;
  last_seconds = seconds;
  
  delay(50);
  
}

void sendData() {
//  if (WiFi.status() != WL_CONNECTED) {  // dont use this if in forced sleep
//    connectToWifi();
//  }                                     // if in UDP should never have to use this  
  Udp.beginPacket(host, port);  
  Udp.write(data);  // aparently a limit of 1024
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


