#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
extern "C" {
  #include "user_interface.h"
}

ADC_MODE(ADC_VCC);

// wifi
WiFiClient connection;
const char* ssid      = "3V8VC";
const char* password  = "7YYGM8V3R65V52FJ";
const char* host      = "192.168.1.9";
//const char* ssid     = "GL-MT300N-5cb";
//const char* password = "goodlife";
//const char* host     = "192.168.8.235";
const int port        = 8000;

// memory
uint32_t mem;
const int transmit_bytes = 32 * 80;   // 91 appears to be max
String data = "";

// time
unsigned long ms;
unsigned long ms_test;
unsigned long seconds;
unsigned long last_seconds = 0;

// accel
Adafruit_MMA8451 mma = Adafruit_MMA8451();


void setup() {
  Serial.begin(115200);
  delay(1);
  pinMode(2, OUTPUT);  

  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
  
  Wire.begin(12, 14);    // SDA, SCL
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);  

//  data[sizeof(data) - 1] = '\0';
  
//  connectToWifi();
  WiFi.forceSleepBegin();

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

  float mag = sqrt((event.acceleration.x * event.acceleration.x) + (event.acceleration.y * event.acceleration.y) + (event.acceleration.z * event.acceleration.z)) - 9.8; // subtract gravity
  int bat = lround(((ESP.getVcc() - 2724.0) / (3622.0 - 2724.0)) * 100); // constants for adafruit battery, 3.0v-4.2v  
  data += String(ESP.getChipId()) + "," + String(WiFi.RSSI() * -1) + "," + bat + "," + ms + "," + String(mag, 4) + ";";

  if (seconds != last_seconds) {
    Serial.println(data.length());    
  }
  last_seconds = seconds;
  
  if (data.length() >= transmit_bytes) {
    Serial.print("--> ");
    Serial.println(data.length());
    sendData();
  }
  
  delay(50);
}

void sendData() {
  
  mem = system_get_free_heap_size();
  Serial.print("Free memory: ");
  Serial.println(mem);

  Serial.println("Transmitting..."); 

  WiFi.forceSleepWake();
  delay(5000);
  Serial.println("--> awake");

  while (!connection.connect(host, port)) {
    Serial.println("--> connection failed");
    if (WiFi.status() != WL_CONNECTED) {
      connectToWifi();
    }
    if (!connection.connect(host, port)) {
      return;
    }
  }
  Serial.println("--> connection active");

  Serial.print("--> data length: ");
  Serial.println(data.length());

  ms_test = millis();
  connection.println("POST / HTTP/1.1");
  connection.print("Host: ");
  connection.print(host);
  connection.print(":");
  connection.println(port);
  connection.println("Connection: keep-alive");
  connection.println("Content-Type: text/plain");  
  connection.println("Cache-Control: no-cache");  
  connection.print("Content-Length: ");
  connection.println(data.length());
  connection.println();
  
  connection.print(data);          

  unsigned long timeout = millis();
  while (connection.available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println("--> timeout!");
      connection.stop();
      break;
    }
  }

  String line = connection.readStringUntil('\r');
  Serial.print("--> ");
  Serial.println(line);
  connection.stop();  

  Serial.print("--> done (");
  Serial.print(millis() - ms_test);
  Serial.println("ms)");
  Serial.println();

  WiFi.forceSleepBegin();

  data = "";
  
}

void connectToWifi() {
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i % 10 == 0) {
      Serial.println();
      Serial.print("--> attempting to connect to: ");
      Serial.println(ssid);      
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    digitalWrite(2, HIGH);
    delay(250);
    digitalWrite(2, LOW);
    delay(250);      
    Serial.print(".");
    i++;
  }
  Serial.println();
  Serial.println("--> connected to wifi");
}


