# Cheerlights Clock

This project was seen on Twitter, as a interesting 24 hour clock, that changed colors based on the cheerlight color.  
The original project is by @martinbateman, and was shared by @cymplecy.  
The project took on a life of its own after a few tweets back and forth, and quickly turned into a full fledge collabration.  

## Original Project code

The original project code can be found on pastebin.  
https://t.co/1gRc56wNOE  
https://pastebin.com/4Ec6d4xY  
Version 6 code test, which this is mostly based on: https://pastebin.com/N2bH9m50  

Early revisions:  
https://pastebin.com/BuKqBAK3  
(M5StickC added) https://pastebin.com/1abVSN9Q  
(Weather Brief added) based on version 6 code base https://pastebin.com/BBHRsysN  
(Other Boards added in based code) https://pastebin.com/4Sr17ux2  

Other Revisions: (Maintained by cymplecy):  
https://github.com/cymplecy/TTGO-T_DISPLAY-Cheerlights  

## Working Boards

The default board is for the TTGO T-Display board, comment out the other boards if you are using a T-Display.  
TTGO T2 Board - uses a color OLED, and is smaller, a few changes needed to be made. IF you are using this board, uncomment the #define TTGO_T2.  
M5StickC board was added, because (why not?) - to use this with a M5StickC board, comment the other board, uncomment the #define M5Stick_C 1  
Adding other boards/displays should be easy and fairly straight forward. (Please share if you add other devices.).  
Aug 1, added support for TTGO TS 144 board.  

## Features

The sketch will attempt to guess your location using geolocation, and the openweathermap.org api. It will then attempt to guess your timezone, and set the time.  
You will need an API key from openweathermap.org, the key is put in the data/config.json file. And that file is then uploaded to your board using the SPIFFS file system.  
You will need to install the SPIFFS tool into your Arduino IDE, information and the tool can be found here:  
https://github.com/me-no-dev/arduino-esp32fs-plugin  
download the tool from here: (don't download the repo, download the release.)  
https://github.com/me-no-dev/arduino-esp32fs-plugin/releases  
Once the sketch is loaded on your device, you can use OTA updates to change the config.json file, or to update and change the sketch on the device.  
You will notice a new serial port (a TCP/IP port) when the device is connected to your network, select this port for OTA updates.  
In the sketch, you can select either Celsius display (the current default display). Or Fahrenheit. To use Fahrenheit, uncomment the #define Fahrenheit statement.  
You can also select if you want the display to be 24 Hour time (sometimes called Military Time) or 12 Hour Time. The default is for 24 hour time.  
12 hour time will drop the seconds from being displayed and add AM or PM.  Use the #define HOUR12 for 12 hour time, otherwise comment this line for 24 hour time.  

## Installation/Libraries Used

You will need to have the ESP32 Core installed for starters. https://github.com/espressif/arduino-esp32  

I beleive you should beable to find this libraries in the Arduino library Manager, links are provided where we know them.  
(or provided in the IDE by default.)  

Timelib Maintainer Paul Stoffregen. http://playground.arduino.cc/code/time  
PubSubClient https://github.com/knolleary/pubsubclient  

For the TTGO T2 board.  
Adafruit_GFX and Adafruit_SSD1331 are needed, both can be found in the library manager. (I believe).  

For the M5StickC Install the M5StickC library from https://github.com/m5stack/M5StickC  

The TTGO T-Display requires the TFT_eSPI library https://github.com/Bodmer/TFT_eSPI  

TTGO TS 144 display requires Adafruit_GFX and Adafruit_ST7735 library, both should be in the library manager.  

## Revision History

v6.9 Aug 1, 2019 - added support for TTGO_TS_144 Board, changed how the weather and geolocation work (no longer grab lat/lon on each loop), added some spaces after weather brief.  


## Things that will change (maybe)

Currently you need to set your SSID and Password, look for "replace with your network credentials" line in the sketch.  
Plans are either to add this to the config.json or to use a wifi manager for first connection.  

## Things To Do

M5StickC has a real time clock, code should set that and use it.  
Wifi Manager, or ESP Touch, or maybe SSID and Password to config.json file  

Open to additional ideas.  

## Usage

## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request

## Support Me

If you find this or any of my projects useful or enjoyable please support me.  
Anything I do get goes to buy more parts and make more/better projects.  
https://www.patreon.com/kd8bxp  
https://ko-fi.com/lfmiller  
https://www.paypal.me/KD8BXP  

## Other Projects

https://www.youtube.com/channel/UCP6Vh4hfyJF288MTaRAF36w  
https://kd8bxp.blogspot.com/  


## Credits

Martin Bateman,   
@cymplecy  
LeRoy Miller  

## License


