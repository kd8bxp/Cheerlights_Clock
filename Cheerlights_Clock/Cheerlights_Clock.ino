/*
  Cheerlights Clock v7
  based on @martinbateman clock code
 https://t.co/1gRc56wNOE
 https://pastebin.com/4Ec6d4xY
 and updated v6 test code
 https://pastebin.com/N2bH9m50

added cheerlight code back in from v6 above. - LeRoy Miller July 27
added TTGO_T2 and M5SickC code to main code base using defines

ported to TTGO T2 Board by LeRoy Miller July 25, 2019

added Geolocate Timezone code July 25, 2019
ported to M5StickC LeRoy Miller July 26, 2019
added support for TTGO_TS_144 device Aug 1, 2019

July 27 - updated M5StickC display to display weather description and temp.
 reformated screen for better display of time, and color and to make room for weather info.
 reformated TTGO_T2 screen to fit brief weather and temps.
 
  TO DO: add M5StackC Real Time Clock support
  
  * Idea add SPIFFs support, OTA support (Added July 30, 2019 LeRoy Miller)
  * add a Wifi Manager to update OTA
  * Update API Key with no reprogramming (done OTA update SPIFFS see https://github.com/me-no-dev/arduino-esp32fs-plugin for info on how to install, use https://github.com/me-no-dev/arduino-esp32fs-plugin/releases to get the tool)
  * Make for 24 hour (current) or 12 hour time (done July 30, 2019 LeRoy Miller)
  * Make temperatures either C or F (done July 30, 2019)

Aug 1 - added TTGO_TS_144 board, changed how the weather and geolocation work (no longer update lat/lon on each loop), added some spaces after weather brief.
Aug 5 - added date to TTGO_T_DISPLAY
*/

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "FS.h"
#include "SPIFFS.h"

//Define your board type default to TTGO_T_Display
//#define TTGO_T2 1
//#define M5Stick_C 1
//#define TTGO_TS_144 1 

//Define your temperature units Default will be Celsius
#define Fahrenheit //comment out for Celsius

//Define 24 hour time (default) or 12 hour time
//#define HOUR12 //comment out for 24 hour time

#ifdef TTGO_T2
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1331.h>
  #define sclk 14
  #define mosi 13
  #define cs   15
  #define rst  4
  #define dc   16
  #define TFT_BL          -1  // Display backlight control pin
  // Color definitions
  #define TFT_BLACK           0x0000
  #define TFT_BLUE            0x001F
  #define TFT_RED             0xF800
  #define TFT_GREEN           0x07E0
  #define TFT_CYAN            0x07FF
  #define TFT_MAGENTA         0xF81F
  #define TFT_YELLOW          0xFFE0
  #define TFT_WHITE           0xFFFF
  Adafruit_SSD1331 tft = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);
#elif M5Stick_C  
  #include <M5StickC.h>
  #define TFT_BL          -1  // Display backlight control pin
  // Color definitions
  #define TFT_BLACK           0x0000
  #define TFT_BLUE            0x001F
  #define TFT_RED             0xF800
  #define TFT_GREEN           0x07E0
  #define TFT_CYAN            0x07FF
  #define TFT_MAGENTA         0xF81F
  #define TFT_YELLOW          0xFFE0
  #define TFT_WHITE           0xFFFF
#elif TTGO_TS_144
  #include <Adafruit_GFX.h> 
  #include <Adafruit_ST7735.h> 
  #define TFT_CS 16
  #define TFT_RST 9
  #define TFT_DC 17
  #define TFT_SCLK 5   // set these to be whatever pins you like!
  #define TFT_MOSI 23   // set these to be whatever pins you like!
  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
  #define TFT_BL          -1  // Display backlight control pin
#else
  #include <TFT_eSPI.h>
  #define TFT_BL          4  // Display backlight control pin
  TFT_eSPI tft = TFT_eSPI();
#endif

// Replace with your network credentials
const char* ssid     = "";
const char* password = "";
const char* mqtt_server = "simplesi.cloud";
// openweathermap.org key
String WEATHERKEY;

// temp in celcius
double temperature = 0.0;

//Weather Statement
String weatherStatement;

// offset in seconds
int gmtOffset_sec = 0;
//Original Code
int cheer_red = 0;
int cheer_green = 0;
int cheer_blue = 0;
unsigned int rgb565Decimal = 0x8410;
unsigned int newrgb565Decimal;
String colourString = "";
String newColourString;

String strData;
String topicStr;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WiFiClient espClient;
PubSubClient client(espClient);

String payload;
double lat, lon;

SemaphoreHandle_t serialMutex = NULL;
void updateNTP (void *pvParams);
void updateScreen (void *pvParams);

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  topicStr = topic;

  Serial.print("Message:");
  
  strData = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    strData += (char)payload[i];
  }
  
  Serial.println();
  Serial.println("-----------------------");

  if (topicStr.endsWith("cheerlights/rgb565Decimal")) {
   
    colourString = newColourString;
    rgb565Decimal = strData.toInt();
    Serial.println("*******");
  
    Serial.println(rgb565Decimal);
  }  
  if (topicStr.endsWith("cheerlights")) {
   
    newColourString = "\n" + strData + "                "; //newColourString = "Cheerlights:\n" + strData;
    //sixteenBitHex = newSixteenBitHex;
    Serial.println(strData);
  }  
} // end callback

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("cheerlights",1);
      client.subscribe("cheerlights/rgb565Decimal",1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);

  serialMutex = xSemaphoreCreateMutex ();

  if (TFT_BL > 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
  }
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  if (!loadConfig()) {
    Serial.println("Failed to load config");
  } else {
    Serial.println("Config loaded");
  }

#ifdef TTGO_T2
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
#elif M5Stick_C
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
#elif TTGO_TS_144
  tft.initR(INITR_144GREENTAB);
  tft.setRotation(3); 
  tft.fillScreen(ST7735_BLACK); 
#else
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); // Note: the new fonts do not draw the background colour
#endif

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print (".");
  }
  Serial.println(WiFi.localIP());

//OTA Code
ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();


  timeClient.begin();
  geolocation(); // guess where I am
  getWeather(); //because we want weather, and dont need to update or lat/lon or timezone each loop.
  timeClient.setTimeOffset(gmtOffset_sec);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  xTaskCreate (updateNTP, "NTP Client", 4096, NULL, 2, NULL);
  xTaskCreate (updateScreen, "Screen", 4096, NULL, 1, NULL);
}

void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
}

void getJson(String url) {  
   if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
     HTTPClient http;  //Declare an object of class HTTPClient
     http.begin(url);  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
     if (httpCode > 0) { //Check the returning code
       payload = http.getString();   //Get the request response payload
     
    }
 
    http.end();   //Close connection
 
  }
}

/*
 * This function will use the external IP to get a Latitude and Longitude 
 * which will be used for many different things for ISS tracker.
 * The website http://ip-api.com is used for this.
 * https://github.com/kd8bxp/M5Stack-ISS-Tracker-Updated/tree/master/M5Stack_ISS_Tracker_Updated
 */

int geolocation(){
  String url = "http://ip-api.com/json";
  getJson(url);
 
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument jsonBuffer(1024);
  DeserializationError error = deserializeJson (jsonBuffer, payload);
  if (error){
      Serial.println ("deserializeJson () failed");
  }
  lat = jsonBuffer["lat"];
  lon = jsonBuffer["lon"];
}

 void getWeather() { 
 String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lon) + "&appid=" + WEATHERKEY;
  Serial.println(url);
  getJson(url);
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonDocument jsonBuffer(1024);
  DeserializationError error = deserializeJson (jsonBuffer, payload);
  error = deserializeJson (jsonBuffer, payload);
  if (error){
      Serial.println ("deserializeJson () failed");
  }
  gmtOffset_sec = jsonBuffer["timezone"]; //maybe causing a problem on new calls (?) may only need this one time (?)
  temperature = jsonBuffer["main"]["temp"];
  temperature = temperature - 273.0;
  //JsonObject obj = jsonBuffer.as<JsonObject>();
  
  weatherStatement = jsonBuffer["weather"][0]["description"].as<String>();
  weatherStatement += "              ";
  Serial.println(temperature);
  Serial.println(weatherStatement);
}

void updateNTP (void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    if (xSemaphoreTake (serialMutex, (TickType_t)10) == pdTRUE) {
      setTime (timeClient.getEpochTime ());
      xSemaphoreGive (serialMutex);
    }
    vTaskDelay ((1000/portTICK_PERIOD_MS) * 60 * 15);  // update every 15 minutes.
    getWeather(); //geolocation (); // update based on location
  }
}

void updateScreen (void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    if (xSemaphoreTake (serialMutex, (TickType_t)10) == pdTRUE) {
      char timeString[25];
      char colourString2[25];
      
      colourString.toCharArray(colourString2,25);
      
      time_t t = now ();
   
#ifdef HOUR12
  #if defined (TTGO_TS_144) || defined (M5Stick_C)
  sprintf (timeString, "%02i:%02i:%02i", hourFormat12(), minute (t), second (t));
  #else
  sprintf(timeString, "%02i:%02i ", hourFormat12(), minute(t));
  #endif
  if (isAM()) { 
    String temp = "AM";
    strcat (timeString , temp.c_str()); } else {
    String temp = "PM";
    strcat(timeString, temp.c_str());}
#else     
      sprintf (timeString, "%02i:%02i:%02i", hour (t), minute (t), second (t));
#endif
      xSemaphoreGive (serialMutex);
char out[25];
#ifdef Fahrenheit
      sprintf (out,"%2.0f`F", (temperature * 9/5)+32); 
#else      
      sprintf (out, "%2.0f`C", temperature);
#endif

String dayArray[9] = {"","Sun","Mon","Tue","Wed","Thr","Fri","Sat"};
String monthArray[14] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
String dateString = dayArray[weekday()] + " " + monthArray[month()] + " " + day() + ", " + year() + " ";
//Serial.print("Date: "); Serial.print(dayArray[weekday()]); Serial.print(" "); Serial.print(monthArray[month()]); Serial.print(" ");Serial.print(day()); Serial.print(", "); Serial.println(year());

#ifdef TTGO_T2
//some screen flashing when updating (?)
      tft.setTextSize(2);
      tft.setTextColor(0x39C4, TFT_BLACK);
      tft.setTextColor(rgb565Decimal, TFT_BLACK);
      tft.setCursor(0,0);
      tft.print(timeString); 
      tft.setTextSize(1);
      tft.setCursor(0,17);
      tft.print(dateString);
      //tft.setCursor(0,4);
      //tft.setTextSize(2);
      //tft.print(colourString2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(1);
      tft.setCursor(0,42);
      tft.print(weatherStatement);
      tft.setCursor(40,52);
      tft.print(out);
#elif M5Stick_C
      M5.Lcd.setTextColor(0x39C4, TFT_BLACK);
      #ifdef HOUR12
      M5.Lcd.setCursor(0,0,4);
      #else
      M5.Lcd.setCursor(25,0,4);
      #endif
      M5.Lcd.setTextColor(rgb565Decimal, TFT_BLACK);
      M5.Lcd.print(timeString); //tft.drawString (timeString, 10, 10, 7);
      M5.Lcd.setCursor(20,25,2);
      M5.Lcd.print(dateString);
      //M5.Lcd.setCursor(0,1,4);
      //M5.Lcd.print(colourString2);
      M5.Lcd.setCursor(0,60,2);
      M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.print(weatherStatement);
      M5.Lcd.setCursor(118,60,2);
      //M5.Lcd.setTextColor(TFT_WHITE);
      M5.Lcd.print(out);
#elif TTGO_TS_144
      tft.setTextSize(2);
      tft.setTextColor(0x39C4, ST7735_BLACK);
      tft.setTextColor(rgb565Decimal, ST7735_BLACK);
      tft.setCursor(0,0);
      tft.print(timeString); 
      tft.setTextSize(1);
      tft.setCursor(12,17);
      tft.print(dateString);
      //tft.setCursor(0,4);
      //tft.setTextSize(2);
      //tft.print(colourString2);
      tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
      tft.setTextSize(1);
      tft.setCursor(0,42);
      tft.print(weatherStatement);
      tft.setCursor(40,72);
      tft.print(out);
#else      
      tft.setTextColor(0x39C4, TFT_BLACK);
      #ifndef HOUR12
      tft.drawString("88:88:88",10,10,7);
      #endif
      tft.setTextColor(rgb565Decimal, TFT_BLACK);
      tft.drawString (timeString, 10, 10, 7);
      tft.drawString(dateString,25,63, 4);
      tft.setTextColor(TFT_WHITE);
      tft.setCursor(0,100,4);
      tft.print(weatherStatement);
      tft.setCursor(175, 100 ,4); 
      tft.println(out);
#endif
      
    }
    vTaskDelay (1000/portTICK_PERIOD_MS);
  }
}

bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
Serial.println(buf.get());
const size_t capacity = JSON_OBJECT_SIZE(1) + 50;
  DynamicJsonDocument jsonBuffer(capacity);
  DeserializationError error = deserializeJson (jsonBuffer, buf.get());
  if (error){
      Serial.println ("deserializeJson () failed");
  }

WEATHERKEY = jsonBuffer["WEATHERKEY"].as<String>();
 
  return true;
}
