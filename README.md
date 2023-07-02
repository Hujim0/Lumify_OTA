# SmartLights_with_ESP
 
Control LED Strip from your server on local network

USED HARDWARE:
- LED strip WS2812B
- main board: ESP 12-E
- external DC that would power your whole system 

HARDWARE CONNECTIONS:
- connect esp, led strip and power supply grounds (to stop random LED strip flickering)
- connect your DC output -> VIN pin on your ESP and to power line on LED strip
- connect LED data wire -> ESP D4 (with 200 Ohm resistor to reduce noise)
- put a 1000uF capacitor between your LED strip's power and ground lines (to help smooth out a power supply) 
- put a 100uF capacitor between your ESP's 3v pin and GND pin (to smooth out power supply)

DC:
- WS2812B uses 5 Volt logic and 60 mAmps for one LED at full brightness with a white color,
you should get a DC that can output needed for strip power.

for 600 LED strip you should get 600 * 0.06 = 36 Amps, 5 volts DC minimum.
I suggest you buying a DC with 2-5 spare Amps that would power your ESP.

---------------------------------------------------------------------------------------------------

TO DEVELOP:

library dependencies:
- PlatformIO
- FastLED
- ArduinoJSON
- AsyncTCP
- AsyncServer
- LittleFS

CONFIGURE:
- go to data/wifi_settings.txt and change first and second lines to network name and password, dont forget that name and password should go on diffrent lines
- read Serial monitor to debug
- go to data/index.html to change your page, dont forget to build your system image through SPIFFS
