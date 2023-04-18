/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/
#include <FastLED.h>
#include <Wifi.h>
#include <NTPClient.h>
#include <WifiUdp.h>

#define r1      12
#define r2      13
#define r3      11
#define ledPin  15
#define numLed  300

const char* ssid        = "PineappleNation";
const char* password    = "whatdoyouthink?";

CRGB led[numLED];
RTC_DATA_ATTR int readingID = 0;

String date;
String day;
String time;
WifiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int lightHour   = 16;
int startHour   = 8;
int waterSecond = 5;
int waterFreq   = 1;//per Hour


//ok how do i approach this do i give a time run or do i set the on off time
// the setup function runs once when you press reset or power the board
void onLED()
{
  for(int x = 0; x>numLed, x++)
  {
    led[x]=CRGB(255,10,128);
  }
  FastLED.show();
}
void offLED()
{
  FastLED.clear();
}
void waterOn()
{
  digitalWrite(r2,HIGH);
  digitalWrite(r1,HIGH);
}
void waterOff()
{
  digitalWrite(r1,LOW);
  digitalWrite(r2,LOW);
}
void startTime()
{
  timeClient.begin();
  timeClient.setTimeOffset(28800);
}
void updateTime()
{
  while(!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  date = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOF("T");
  day = date.substring(0,splitT);
  Serial.println(day);
  time = date.substring(splitT+1, date.length()-1);
  Serial.println(time);
}
void connectWifi()
{
  int attempt = 0;
  Serial.print("Connecting to: "");
  Serial.println(ssid);
  Wifi.begin(ssid,password);
  while((Wifi.status()!=WL_CONNECTED)&&attempt<5)
  {
    delay(500);
    Serial.println("connecting");
    attempt ++;
  }
  Serial.println("Access Interface Via");
  Serial.println(Wifi.localIP());
}
void sendData()
{
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  connectWifi();
  startTime();
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);
  FastLED.addLeds<WS2812, ledPin, RGB>(led, numLed);
}

// the loop function runs over and over again forever
void loop()
{
  updateTime();

}
