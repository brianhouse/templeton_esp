This works.

ESP-07 and Adafruit FTDI Friend



VCC -> 3.3v
GND -> GND
TX -> RX
RX -> TX
EN/CH_PD -> 3.3v

for load mode:
GPIO0 -> GND

for ESP-07:
GPIO15 -> GND


To read battery on ADC, leave it unconnected. then `ADC_MODE(ADC_VCC)` at top of sketch (outside all functions), and `ESP.getVcc()` thereafter.


To read a simple switch between VCC and ground on GPIO 14:

    pinMode(14, INPUT_PULLUP);
    digitalRead(14);


To read I2C:

    Wire.begin(SDA, SCL)

SDA is GPIO12
SCL is GPIO14

/


5000 mah Lipo would last. Just under 60 hours till it stopped transmitting. - See more at: http://www.esp8266.com/viewtopic.php?f=13&t=8185#sthash.5KtwUvDf.dpuf

https://github.com/esp8266/Arduino