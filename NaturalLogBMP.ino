/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <FastLED.h>

Adafruit_BME280 bme; // I2C
/***************************************************************************
#define ledPin        14
#define numLeds       300
#define brightness    255
#define ledType       WS2812
#define colorOrder    GRB
CRGB leds[numLeds];
int r = 255;
int g = 80;
int b = 40;


#define solenoidPin   25
#define pumpPin       26
#define atomizerPin   27
#define soilPin       34

String ledState = "";
String pumpState = "";
String atomizerState = "";
//Adafruit_BMP280 bmp;
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

// Replace with your network credentials
****************************************************************************/
const char* ssid = "PinappleNation";
const char* password = "whatdoyouthink?";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readBMP280Temperature()
{
  // Read temperature as Celsius (the default)
  float t = bme.readTemperature();
  // Convert temperature to Fahrenheit
  //t = 1.8 * t + 32;
  if (isnan(t))
  {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else
  {
    Serial.println(t);
    return String(t);
  }
}

String readBME280Humidity()
{
  float h = bme.readHumidity();
  if (isnan(h))
  {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else
  {
    Serial.println(h);
    return String(h);
  }
}

String readBME280Pressure() {
  float p = bme.readPressure() / 100.0F;
  if (isnan(p)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}
/*
String readbmP() {
  float p = bmp.readPressure() / 100.0F;
  if (isnan(p)) {
    Serial.println("Failed to read from BMP280 sensor!");
    return "";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}
String readbmT() {
  float t = bmp.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from BMP280 sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}
*/
String readCCS811CO2(){
  double c = ccs.geteCO2();
  if (isnan(c)) {
    Serial.println("Failed to read from CCS811 sensor!");
    return "";
  }
  else {
    Serial.println(c);
    return String(c);
  }
}

String readCCS811VOC(){
  double v = ccs.getTVOC();
  if (isnan(v)) {
    Serial.println("Failed to read from CCS811 sensor!");
    return "";
  }
  else {
    Serial.println(v);
    return String(v);
  }
}
/*
String processor(const String& var)
{
  Serial.println(var);
  if(var == "stateLight"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  else if (var == "statePump"){
    if(digitalRead(pumpPin)){
      pumpState = "ON";
    }
    else{
      pumpState = "OFF";
    }
    Serial.print(pumpState);
    return pumpState;
  }
  else if (var == "stateAtomizer"){
    if(digitalRead(atomizerPin)){
      atomizerState = "ON";
    }
    else{
      atomizerState = "OFF";
    }
    Serial.print(atomizerState);
    return atomizerState;
  }

}
************************************************************************/
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  /***********************************************************************************************
  CRGB leds[numLeds];
  FastLED.addLeds<ledType, ledPin, colorOrder>(leds, numLeds).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  brightness  );
  pinMode(solenoidPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(atomizerPin, OUTPUT);
  *************************************************************************************************/
  bool status;
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }
  /*
  status = ccs.begin(0x5A);
  if (!status) {
    Serial.println("Could not find a valid CCS811 sensor, check code and wiring!");
  }
*/
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  /************************************************************************************
  //LIGHT SWITCH  //LIGHT SWITCH  //LIGHT SWITCH  //LIGHT SWITCH  //LIGHT SWITCH
  server.on("/lightOn", HTTP_GET, [](AsyncWebServerRequest *request){
    for( int i = 0; i < numLeds; i++)
      {
        leds[i] = CRGB(r,g,b);
        FastLED.show();
      }
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/lightOff", HTTP_GET, [](AsyncWebServerRequest *request){
      FastLED.clear();
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  //LIGHT SWITCH  //LIGHT SWITCH  //LIGHT SWITCH  //LIGHT SWITCH  //LIGHT SWITCH
  //PUMP SWITCH  //PUMP SWITCH  //PUMP SWITCH  //PUMP SWITCH  //PUMP SWITCH
  server.on("/pumpOn", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(pumpPin, HIGH);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/pumpOff", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(pumpPin, LOW);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  //PUMP SWITCH  //PUMP SWITCH  //PUMP SWITCH  //PUMP SWITCH  //PUMP SWITCH
  //ATOMIZER SWITCH  //ATOMIZER SWITCH  //ATOMIZER SWITCH  //ATOMIZER SWITCH
  server.on("/atomizerOn", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(atomizerPin, HIGH);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/atomizerOff", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(atomizerPin, LOW);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  //ATOMIZER SWITCH  //ATOMIZER SWITCH  //ATOMIZER SWITCH  //ATOMIZER SWITCH
****************************************************************************************************/
  Serial.println("request temperature");
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Temperature().c_str());
  });
  Serial.println("request humidity");
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Humidity().c_str());
  });
  Serial.println("request pressure");
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readBME280Pressure().c_str());
  });
  /***************************************************************************************************
  Serial.println("request co2");
  server.on("/CO2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readCCS811CO2().c_str());
  });
  Serial.println("request voc");
  server.on("/VOC", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readCCS811VOC().c_str());
  });
  *****************************************************************************************************/
  /****************************************************************************************************
  Serial.println("request bmP");
  server.on("/bmP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readbmP().c_str());
  });
    Serial.println("request bmT");
  server.on("/bmT", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readbmT().c_str());
  });
  */
  // Start server
  server.begin();
}

void loop()
{
  /*
 if(ccs.available()){
    float temp = ccs.calculateTemperature();
    if(!ccs.readData()){
      Serial.print("Temp: ");
      Serial.println(temp);
      Serial.print("CO2: ");
      Serial.print(ccs.geteCO2());
      Serial.print("ppm, TVOC: ");
      Serial.print(ccs.getTVOC());
      Serial.print("ppb");
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  }
  delay(5000);
  */

}
