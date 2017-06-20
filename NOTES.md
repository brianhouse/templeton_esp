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


See available memory:

    uint32_t free = system_get_free_heap_size();


Sleeping the modem:


up top:

    extern "C" {
      #include "user_interface.h"
    }

then

    WiFi.forceSleepBegin();

then

    WiFi.forceSleepWake();

Begin is instantaneous, but Wake takes four seconds or so.


in theory, it's already shutting the modem down when it doesnt need it. I guess force makes it ignore send and receive messages that would ordinarily wake it. For this application, I think it makes more sense to concentrate on pooling requests than to fiddle with turning it off and on.


/

https://github.com/esp8266/Arduino
https://arduino-esp8266.readthedocs.io/en/latest/
http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html
http://esp8266.github.io/Arduino/versions/2.1.0-rc1/doc/libraries.html

on the feather, I'm getting 3.027 for battery, and that has a regulator. so add something to compensate.

http://www.esp8266.com/wiki/doku.php?id=esp8266_power_usage

multimeter reads 4.25 volts on a charged 3.7/150mah.
I'm seeing 3.54 on the battery readout. so who knows.

this thing mysteriously stopped last time, without showing so much of a drop. and then it was 0 on the multimeter, which doesnt make sense.

ok, it dies at 2.54.
so, if you add 0.71, that's 4.25 -> 3.25, which is starting to make sense.
[actually should be 4.2 that is max charge]

but really, it reads 0? [yes, cutoff]

//


47168 free memory / 60 bytes per dataString = 786 rows
786 rows / ~70hz = 10 seconds

we could push this to 30 seconds

750 rows / 25hz = 30 seconds
matches film, anyway

so it sends bursts of data every 30 seconds, hopefully not fucking up the transmission.

doing the math client side, plus some room, is 40chars.

1179 rows

can get this to 45 seconds

//

43848 free memory with 33 x 100
45496 free memory with 33 x 50

33x50 should transmit 1650 bytes, but I'm getting 1024, so that feels like a limit.
31 rows



// ID(8) Bat(2) Rec(2) Time(8) Mag(6) = 26 + ,(4) ;(1) + \0(1) = 32 bytes  
13019021,70,43,86400000,7.1900;\0