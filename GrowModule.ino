#include <FastLED.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define r1      12
#define r2      13
#define r3      11
#define ledPin  15
#define numLed  300

const char* ssid        = "PineappleNation";
const char* password    = "whatdoyouthink?";

CRGB led[numLed];
RTC_DATA_ATTR int readingID = 0;

String date;
String day;
String times;
int currentTime;
int onTime = 6*3600;//time in seconds to turn on
int offTime = 0;// time in seconds to turn off
bool ledState = true;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

uint64_t uS_TO_S_FACTOR = 1000000;
uint64_t TIME_TO_SLEEP = 60;

int lightHour   = 16;
int startHour   = 8;
int waterSecond = 5;
int waterFreq   = 1;//per Hour


//ok how do i approach this do i give a time run or do i set the on off time
// the setup function runs once when you press reset or power the board
void onLED()
{
  Serial.println("LED Power On Sequence");
  for(int x = 0; x<numLed; x++)
  {
    led[x]=CRGB(255,10,128);
   // Serial.print(x);
  }
  FastLED.show();
}
void offLED()
{
  Serial.println("LED Power Off Sequence");
  FastLED.clear();
  FastLED.show();
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
  timeClient.setTimeOffset(-28800);
}
void updateTime()
{
  while(!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  date = timeClient.getFormattedTime();
  int splitT = date.indexOf("T");
  day = date.substring(0,splitT);
  Serial.println(day);
  times = date.substring(splitT+1, date.length());
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
void sendData()
{
  Serial.println();
}
int timeToSeconds(String inTime)
{
  int hour = inTime.substring(0,2).toInt();
  Serial.print("hour in seconds: ");
  Serial.println(hour);
  int minute = inTime.substring(3,5).toInt();
  Serial.print("minute in seconds: ");
  Serial.println(minute);
  int second = inTime.substring(6,8).toInt();
  Serial.print("second in seconds: ");
  Serial.println(second);

  int tempSeconds = hour*3600 + minute*60 + second;
  /*int startInd = 0;
  int exp = 3;
  int tempSeconds = 0;
  for(int x = 0; x<inTime.length();x++)
  {
    if(inTime[x].valueOf()==":")
    {
      //x is the index of the :
      tempSeconds += pow(60,exp)*toInt(inTime.substring(startInd, x));
      exp--;
      startInd=x+1;
    }
  }*/
  Serial.print("In Seconds: ");
  Serial.println(tempSeconds);
  return tempSeconds;
}
void checkLightSchedule()
{
  if(currentTime>offTime&&currentTime<onTime)//off time ---- current time ---- on time
  {
    ledState = false;
    Serial.println("LED STATE entering false");
  }
  else
  {
    ledState = true;
    Serial.println("LED STATE entering true");

  }
  Serial.println("LED STATE IS : " + ledState);
}
void setup()
{
  Serial.begin(115200);
  connectWifi();
  startTime();
  updateTime();
  checkLightSchedule();
  // initialize digital pin LED_BUILTIN as an output.
  /*pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);*/
  FastLED.addLeds<WS2812, ledPin, GRB>(led, numLed);
  if(ledState)
  {
    Serial.println("Turning on LED");
    onLED();
  }
  else
  {
    Serial.println("Turning off LED");
    offLED();
  }
  Serial.println(ledState);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //esp_deep_sleep_start(); 
}

// the loop function runs over and over again forever
void loop()
{
  updateTime();
  checkLightSchedule();
    if(ledState)
  {
    Serial.println("Turning on LED");
    onLED();
  }
  else
  {
    Serial.println("Turning off LED");
    offLED();
  }
  delay(60000);
}
