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

// wifi
const char* ssid      = "3V8VC";
const char* password  = "8GGY42ZJRj10550F";
const char* host      = "192.168.1.7";
//const char* ssid     = "GL-MT300N-5cb";
//const char* password = "goodlife";
//const char* host     = "192.168.8.235";
const int port        = 23232; // both send and receive

// memory
uint32_t mem;
const int entry_size = 32;  // includes termination bit
const int batch_size = 32;  // these have to multiply to <= 1024 for UDP
const int set_size = 24;     // total has to keep under available memory (40 is too high)
int entry = 0;
int batch = 0;
char data[set_size][batch_size][entry_size];

// Time
unsigned long ms;
unsigned long ms_test;

bool light_on = false;


void setup() {
  Serial.begin(115200);
  delay(10);
  //  pinMode(2, OUTPUT);

  Serial.print("ID: ");
  Serial.println(ESP.getChipId());

  Wire.begin(2, 14);    // SDA, SCL
  mma.begin();
  mma.setRange(MMA8451_RANGE_2_G);

  connectToWifi();
  Udp.begin(port);
}

void loop() {
  ms = millis() % 86400000; // wrap every day

  // blink the light
  if (ms / 1000 % 2 == 0) {
    if (!light_on) {
      Serial.println("ON");
      //      digitalWrite(2, LOW);
      light_on = true;
    }
  } else {
    if (light_on) {
      Serial.println("OFF");
      //      digitalWrite(2, HIGH);
      light_on = false;
    }
  }

  // read the accelerometer
  mma.read();
  sensors_event_t event;
  mma.getEvent(&event);

  // calculate magnitude
  Serial.print(event.acceleration.x);
  Serial.print(event.acceleration.y);
  Serial.print(event.acceleration.z);
  Serial.println();
  float mag = sqrt((event.acceleration.x * event.acceleration.x) + (event.acceleration.y * event.acceleration.y) + (event.acceleration.z * event.acceleration.z)) - 9.8; // subtract gravity
  if (mag >= 10.0) {
    mag = 9.999;
  } else if (mag <= -10.0) {
    mag = -9.99;
  }

  // get battery
  int bat = lround(((ESP.getVcc() - 2724.0) / (3622.0 - 2724.0)) * 100); // constants for adafruit battery, 3.0v-4.2v
  if (bat == 100) {
    bat = 99; // need this to be 2 bytes max
  }

  // build entry string
  // ID(8) Bat(2) Rec(2) Time(8) Mag(6) = 26 + ,(4) ;(1) + \0(1) = 32 bytes
  // 13019021,70,43,86400000,-7.190;\0
  String dataString = String(ESP.getChipId()) + "," + String(WiFi.RSSI() * -1) + "," + bat + "," + ms + "," + String(mag, 3) + ";";
  dataString.toCharArray(data[batch][entry], dataString.length() + 1);

  // calc batches
  entry++;
  if (entry == batch_size) {
    batch++;
    entry = 0;
  }
  if (batch == set_size) {
    sendData();
    batch = 0;
  }

  delay(50);

}

void sendData() {
  //  if (WiFi.status() != WL_CONNECTED) {  // dont use this if in forced sleep
  //    connectToWifi();
  //  } // if in UDP should never have to use this  [is that true?]
  Serial.println(WiFi.status() == WL_CONNECTED);

  mem = system_get_free_heap_size();
  Serial.print("Free memory: ");
  Serial.println(mem);

  Serial.print("Transmitting...");
  ms_test = millis();
  for (int b = 0; b < set_size; b++) {
    Udp.beginPacket(host, port);      // limiting to 1024
    for (int e = 0; e < batch_size; e++) {
      Udp.write(data[b][e]);
    }
    Udp.endPacket();
    delay(2);
  }
  Serial.print(" done (");
  Serial.print(millis() - ms_test);
  Serial.println("ms)");
}

void connectToWifi() {
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i % 20 == 0) {
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
  digitalWrite(0, HIGH);
}


