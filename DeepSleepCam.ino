#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#define EEPROM_SIZE 1          // define the number of bytes you want to access

//#include "dl_lib.h"


#include <Adafruit_BME280.h>
#include <Adafruit_CCS811.h>
#include <Wire.h>
#include <FastLED.h>
#define GPIO_SDA          15
#define GPIO_SCL          14
Adafruit_CCS811 ccs;
Adafruit_BME280 bme;
float temperature;
float humidity;
float pressure;
float co2;

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
int pictureNumber = 0;
String path;

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
String date;
String day;
String times;
int currentTime;
int onTime = 6*3600;//time in seconds to turn on
int offTime = 0;// time in seconds to turn off
bool ledState = true;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
RTC_DATA_ATTR int readingID = 0;
const char* ssid        = "PineappleNation";
const char* password    = "whatdoyouthink?";

#define ledPin            16
#define numLeds           300
CRGB leds[numLeds];
int red = 255;
int blue = 128;
int green = 0;


#define uS_TO_S_FACTOR 1000000
void camInit()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  ////////////////////////////////////
  config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 10;
  config.fb_count = 2;
  /*
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
*/
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  delay(200);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}
void sdInit()
{
  Serial.println("Starting SD Card");
  delay(500);
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
}
/*
void checkFile()
{
  fs::FS &fs = SD_MMC;
  File file = fs.open("/grow.log");
  if(!file)
  {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD_MMC, "/grow.log", "ImageDir,D TimeStamp,H Humidity,P Pressure,T Temperature \r\n");
  }
  else
  {
    Serial.println("File already exists");
  }
  file.close();
}
void writeFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}*/
void logSDCard()
{
  String dataMessage = path + ",D" + String(times) + ",H" + String(humidity) + ",P" + String(pressure) + ",T" +
                String(temperature) + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD_MMC, "/grow.log", dataMessage.c_str());
  SD_MMC.end();
}
// Append data to the SD card (DON'T MODIFY THIS FUNCTION)
void appendFile(fs::FS &fs, const char * path, const char * message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
void capImage()
{
  flashOn();
  camera_fb_t * fb = NULL;
  // Take Picture with Camera
  fb = esp_camera_fb_get();
  delay(500);
  if(!fb) {
    Serial.println("Camera capture failed");
    ESP.restart();
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  path = String(pictureNumber) +".jpg";

  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb);
  fb = NULL;

  delay(100);
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  //logSDCard();
  SD_MMC.end();
  Serial.println("Picture Taken");
  if(ledState)
  {
    growLightOn();
  }
  else
  {
    growLightOff();
  }
}
void capData()
{
//  Wire.begin(GPIO_SDA,GPIO_SCL);
  Wire.begin(15,14);
  if(!bme.begin(0x76))
  {
    Serial.println("BME not found");
  }
  /*
  if(!ccs.begin())
  {
    Serial.println("CCS not found");
  }*/
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure();
  co2 = ccs.geteCO2();
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  /*
  Serial.print("CO2: ");
  Serial.println(co2);
  */
  Wire.endTransmission();
}
void flashOn()
{
  for(int x = 0; x<numLeds; x++)
  {
    leds[x]=CRGB(255,255,255);
  }
  FastLED.show();
  FastLED.clear();
}
void growLightOn()
{
  if(leds[0]!=CRGB(red,green,blue))
  {
    for(int x = 0; x < numLeds; x++)
    {
      leds[x]=CRGB(red,green,blue);
    }
    FastLED.show();
  }
  FastLED.clear();
}
void growLightOff()
{
  FastLED.clear();
  FastLED.show();
}
void startTime()
{
  timeClient.begin();
  timeClient.setTimeOffset(-28800);
}
void updateTime()
{
  while(!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  date = timeClient.getFormattedDate();
  Serial.println(date);
  int splitT = date.indexOf("T");
/*
  day = date.substring(0,splitT);
  Serial.println(day);
*/
  times = date.substring(splitT+1, date.length()-1);
  Serial.println(times);
  currentTime = timeToSeconds(day);
}
void connectWifi()
{
  int attempt = 0;
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while((WiFi.status()!=WL_CONNECTED))
{
  Serial.println("connecting");
  delay(5000);
}
  Serial.println("Access Interface Via");
  Serial.println(WiFi.localIP());
}
int timeToSeconds(String inTime)
{
  int hour = inTime.substring(0,2).toInt();
  //Serial.print("hour in seconds: ");
  //Serial.println(hour);
  int minute = inTime.substring(3,5).toInt();
  //Serial.print("minute in seconds: ");
  //Serial.println(minute);
  int second = inTime.substring(6,8).toInt();
  //Serial.print("second in seconds: ");
  //Serial.println(second);
  int tempSeconds = hour*3600 + minute*60 + second;
  //Serial.print("In Seconds: ");
  //Serial.println(tempSeconds);
  return tempSeconds;
}
void checkLightSchedule()
{
  bool tempState = ledState;
  if(currentTime>offTime&&currentTime<onTime)//off time ---- current time ---- on time
  {
    ledState = false;
    Serial.println("LED STATE entering false");
    if(tempState != ledState)
    {
      growLightOff();
    }
  }
  else
  {
    ledState = true;
    Serial.println("LED STATE entering true");
    if(ledState != tempState)
    {
      growLightOn();
    }
  }
  Serial.println("LED STATE IS : " + String(ledState));
}
/*
void readConfig()
{
  fs::FS &fs = SD_MMC;
  String configFile = fs.open("grow.cfg");
  if(configFile)
  {
    Serial.println("grow.cfg");
  }
}*/
void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  //readConfig();
  //checkFile();
  camInit();
  connectWifi();
  startTime();
  updateTime();
  checkLightSchedule();
  FastLED.addLeds<WS2812, ledPin, GRB>(leds, numLeds);
}

void loop()
{
  capData();
  delay(200);
  sdInit();
  //logSDCard();
  capImage();
  updateTime();
  checkLightSchedule();
  delay(60000);
}
