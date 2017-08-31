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
const char* password  = "8GGY42ZJRj10550F";
const char* host      = "192.168.1.7";
//const char* ssid     = "GL-MT300N-182";
//const char* password = "goodlife";
//const char* host     = "192.168.8.132";
const int port        = 8000;

// memory
uint32_t mem;
const int transmit_bytes = 32 * 928;   // second terms should be divisble by 32; seems like 928 is max
String data = "";

// time
unsigned long ms;
unsigned long ms_test;
unsigned long seconds;
unsigned long last_seconds = 0;

// accel
Adafruit_MMA8451 mma = Adafruit_MMA8451();

String id;


void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print("ID: ");
  Serial.println(ESP.getChipId());  

  wifi_fpm_set_sleep_type(MODEM_SLEEP_T);

  char idc[9];
  sprintf(idc, "%08d", ESP.getChipId());
  id = String(idc);
  
  Wire.begin(12, 14);    // SDA, SCL              !!!!
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);  

  WiFi.forceSleepBegin();

}

void loop() {
  ms = millis() % 86400000; // wrap every day
  seconds = ms / 1000;
  if (seconds % 2 == 0) {
    digitalWrite(0, LOW);  
  } else {
    digitalWrite(0, HIGH);  
  }

  // ID(8) Bat(2) Rec(2) Time(8) Mag(6) = 26 + ,(4) ;(1) + \0(1) = 32 bytes  

  // accelerometer, 7 bytes
  mma.read();
  sensors_event_t event; 
  mma.getEvent(&event);  
  float mag_value = sqrt((event.acceleration.x * event.acceleration.x) + (event.acceleration.y * event.acceleration.y) + (event.acceleration.z * event.acceleration.z)) - 9.8; // subtract gravity  
  if (mag_value >= 10.0) {
    mag_value = 9.9999;
  }
  if (mag_value <= -10.0) {
    mag_value = -9.9999;
  }
  String mag = String(mag_value, 4);
  if (mag_value >= 0) {
    mag = "+" + mag;
  }

  // battery, 2 bytes
  int bat_value = lround(((ESP.getVcc() - 2724.0) / (3622.0 - 2724.0)) * 100); // constants for adafruit battery, 3.0v-4.2v  
  if (bat_value >= 100) {
    bat_value = 99;
  }
  if (bat_value <= 0) {
    bat_value = 0;
  }
  String bat = String(bat_value);
  if (bat.length() < 2) {
    bat = "0" + bat;
  }

  // time, 8 bytes
  char tv[9];
  sprintf(tv, "%08d", ms);
  String t = String(tv);

  // rssi, 2 bytes
  String rssi = String(abs(WiFi.RSSI()));

  String datum = id + "," + rssi + "," + bat + "," + t + "," + mag + ";";
  
  data += datum;

//  if (seconds != last_seconds) {
//    Serial.println(datum);
//  }
  last_seconds = seconds;
  
  if (data.length() >= transmit_bytes) {
    sendData();
  }
  
  delay(50);
}

void sendData() {
  
  mem = system_get_free_heap_size();
  Serial.print("Free memory: ");
  Serial.println(mem);
  Serial.print("BAT ");
  Serial.println(ESP.getVcc());

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

  for (int i=0; i<(data.length() / 1024) + 1; i++) {
    connection.print(data.substring(i * 1024, (i * 1024) + 1024));   /// this is wrong, somehow
  }

  Serial.println("--> sending...");

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
      Serial.print("Connecting");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
    }
    digitalWrite(0, HIGH);
    delay(250);
    digitalWrite(0, LOW);
    delay(250);      
    Serial.print(".");
    i++;
  }
  Serial.println();
  Serial.println("--> connected");
}


