#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <sunset.h>

#define TIMEZONE  +1
#define LATITUDE  48.9473
#define LONGITUDE 16.2590

const char *ssid     = "Cisco";
const char *password = "Hotentot1919";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "cz.pool.ntp.org", 3600, 60000);
SunSet sun;

unsigned long epochTime = 0;
struct tm *ptm;
int monthDay = 0;
int currentMonth = 0;
int currentYear = 0;
time_t v = 0;
time_t z = 0;
bool isLightOn = false;
String ntpTime = "";
String startTime = ""; 
String testStopTime = "";
String SunriseSunset = "";

void setup()
{
  Serial.begin(115200);
  pinMode(D3, OUTPUT);
  delay(100);
  digitalWrite(D3, LOW);
  delay(100);
  Serial.println("\nSunSet v1.0 starting...");
  Serial.print("Connect to WiFi: ");
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("OK!\n");
  sun.setPosition(LATITUDE, LONGITUDE, TIMEZONE);
  timeClient.begin();
}

void loop() 
{
  if ( WiFi.status() != WL_CONNECTED ) 
  {
    timeClient.end();
    //WiFi.disconnect();
    WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      Serial.print ( "." );
    }
    timeClient.begin();
  }
  else
  {
    Serial.print("\nSignal: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm (-50 db = 100% quality | -100 db = 0% quality)");
    Serial.print("Update NTP time - ");
    timeClient.update();
  
    epochTime = timeClient.getEpochTime();
    ptm = gmtime ((time_t *)&epochTime); 
    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon+1;
    currentYear = ptm->tm_year+1900;
  
    if ( (currentMonth > 3) && (currentMonth < 11) )
    {
      timeClient.setTimeOffset(7200);
      sun.setTZOffset(2);
    }
  
    sun.setCurrentDate(currentYear, currentMonth, monthDay);
    Serial.println(timeClient.getFormattedTime());
  
    v = (time_t)sun.calcCivilSunrise();
    z = (time_t)sun.calcCivilSunset();

    SunriseSunset = "Civil Sunrise - ";
    SunriseSunset += String(minute(v)) + ":" + String(second(v)) + "  || Civil Sunset - " + String(minute(z)) + ":" + String(second(z));
    
    Serial.println(SunriseSunset);

    ntpTime = "";
    startTime = ""; 

    if (timeClient.getHours() < 10 )
    {
      ntpTime += "0";
    }
    ntpTime += timeClient.getHours();
    if (timeClient.getMinutes() < 10 )
    {
      ntpTime += "0";
    }
    ntpTime += timeClient.getMinutes();

    if (minute(z) < 10 )
    {
      startTime += "0";
    }
    startTime += minute(z);
    if (second(z) < 10 )
    {
      startTime += "0";
    }
    startTime += second(z);

    if ( isLightOn == false )
    {
      if ( ntpTime.toInt() >= startTime.toInt() )
      {
        isLightOn = true;
        Serial.println("LightOn = TRUE");
      }
    }

    if ( isLightOn )
    {
      if ( ntpTime.toInt() < startTime.toInt() )
      {
        isLightOn = false;
        Serial.println("LightOn = FALSE");
      }
    }

    if ( isLightOn )
    {
      delay(100);
      digitalWrite(D3, HIGH);
      delay(100);
    }
    else
    {
      delay(100);
      digitalWrite(D3, LOW);
      delay(100);
    }
  
    delay(1000);
  }

}
