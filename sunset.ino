#define DEBUG_NTPClient

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <sunset.h>

#define TIMEZONE  +1
#define LATITUDE  48.9473
#define LONGITUDE 16.2590

const char *ssid     = "DomaN3T";
const char *password = "domanet397117";

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

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
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
  
    String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
    Serial.print("Current date: ");
    Serial.println(currentDate);
  
    v = (time_t)sun.calcCivilSunrise();
    z = (time_t)sun.calcCivilSunset();
  
    Serial.print(hour(v));
    Serial.print(":");
    Serial.print(minute(v));
    Serial.print(":");
    Serial.println(second(v));
    
    Serial.print(hour(z));
    Serial.print(":");
    Serial.print(minute(z));
    Serial.print(":");
    Serial.println(second(z));
  
    delay(1000);
  }
}
