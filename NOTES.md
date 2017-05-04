This works.

ESP-07 and Adafruit FTDI Friend



VCC -> 3.3v
GND -> GND
TX -> RX
RX -> TX
EN -> 3.3v
GPIO15 -> GND

for load mode:
GPIO0 -> GND


/

to read a simple switch on GPIO 14:

    pinMode(14, INPUT_PULLUP);
    digitalRead(14);



/

5000 mah Lipo would last. Just under 60 hours till it stopped transmitting. - See more at: http://www.esp8266.com/viewtopic.php?f=13&t=8185#sthash.5KtwUvDf.dpuf