#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <sunset.h>

#define __SUNSET_DEBUG

#define TIMEZONE  +1
#define LATITUDE  48.9473
#define LONGITUDE 16.2590
#define WTRETRY   20
#define WAITTIME  500
#define NTPSERVER "cz.pool.ntp.org"
#define RELAYPIN  D3

struct WD
{
  String SSID;
  String Password;
};

WD WifiData[3] = {{"MAJI","14042015"},{"DomaNET","domanet397117"},{"DomaN3T","domanet397117"}};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTPSERVER, 3600, 60000);
SunSet sunTime;
time_t sunriseCivilTime;
time_t sunsetCivilTime;
bool lightOn = false;
bool newDate = false;
unsigned long epochTime = 0;
struct tm *ptm;
int currentDay = 0;
int currentMonth = 0;
int currentYear = 0;
bool summerTime = false;
unsigned long startSecond = 0;
unsigned long actSecond = 0;

bool testForConnect()
{
  int waitDelta = 0;
  if ( WiFi.status() == WL_CONNECTED )
  {
    #ifdef __SUNSET_DEBUG
      Serial.print("WIFI IS STILL CONNECTED-> DISCONNECTING...");
    #endif
    WiFi.disconnect();
    while( WiFi.status() == WL_CONNECTED )
    {
      #ifdef __SUNSET_DEBUG
        Serial.print(".");
      #endif
      delay(WAITTIME);
      waitDelta++;
      if ( waitDelta > WTRETRY )
        break;
    }
    #ifdef __SUNSET_DEBUG
      Serial.println("OK!");
    #endif
  }
  #ifdef __SUNSET_DEBUG
    Serial.println("STARTING CONNECT TO WIFI FROM WIFILIST");
  #endif
  for ( int index = 0; index < 3; index++ )
  {
    #ifdef __SUNSET_DEBUG
      Serial.print("CONNECT TO SSID: ");
      Serial.println(WifiData[index].SSID);
      Serial.print("PASSWORD: ");
      Serial.println(WifiData[index].Password);
    #endif
    waitDelta = 0;
    WiFi.begin(WifiData[index].SSID,WifiData[index].Password);
    while ( WiFi.status() != WL_CONNECTED )
    {
      #ifdef __SUNSET_DEBUG
        Serial.print(".");
      #endif
      delay(WAITTIME);
      waitDelta++;
      if ( waitDelta > WTRETRY )
      {
        #ifdef __SUNSET_DEBUG
          Serial.println("TIMEOUT!");
        #endif
        break;
      }
    }
  }
  if ( WiFi.status() != WL_CONNECTED )
  {
    #ifdef __SUNSET_DEBUG
      Serial.println("\nTOTAL TIMEOUT! WIFI NOT CONNECTED!\n");
    #endif
    return false;
  }
  #ifdef __SUNSET_DEBUG
    Serial.println("\nSUCCESS! WIFI IS CONNECTED\n");
  #endif
  return true;
}

bool testForNTP()
{
  int waitDelta = 0;
  #ifdef __SUNSET_DEBUG
    Serial.print("UPDATE TIME FROM NTP ");
    Serial.print(NTPSERVER);
    Serial.print("...");
  #endif

  timeClient.begin();
  
  while( !timeClient.update() )
  {
    #ifdef __SUNSET_DEBUG
      Serial.print(".");
    #endif
    delay(WAITTIME);
    waitDelta++;
    if ( waitDelta > WTRETRY )
    {
      #ifdef __SUNSET_DEBUG
        Serial.println("TIMEOUT!");
      #endif
      break;
    }
  }

  if ( !timeClient.isTimeSet() )
  {
    #ifdef __SUNSET_DEBUG
      Serial.println("NTP UPDATE FAILED!");
    #endif
    timeClient.end();
    return false;
  }

  #ifdef __SUNSET_DEBUG
    Serial.println("SUCCESS!");
  #endif
  return true;
}


void setup()
{
  Serial.begin(115200);
  delay(50);
  pinMode(RELAYPIN, OUTPUT);
  delay(50);
  digitalWrite(RELAYPIN, LOW);
  delay(50);
    #ifdef __SUNSET_DEBUG
      Serial.println("\nSUNSET SWITCH V2 STARTING...\n");
    #endif
  while ( !testForConnect() )
  {
    #ifdef __SUNSET_DEBUG
      Serial.println("..A MOMENT FOR NEW ATTEMPTS!\n");
    #endif
    delay(WAITTIME*WTRETRY);
  }
  while ( !testForNTP() )
  {
    #ifdef __SUNSET_DEBUG
      Serial.println("..A MOMENT FOR NEW ATTEMPTS!\n");
    #endif
    delay(WAITTIME*WTRETRY);
  }
  epochTime = timeClient.getEpochTime();
  ptm = gmtime ((time_t *)&epochTime); 
  currentDay = ptm->tm_mday;
  currentMonth = ptm->tm_mon+1;
  currentYear = ptm->tm_year+1900;
  // vypocet relativniho letniho casu
  if ( (currentMonth > 3) && (currentMonth < 11) )
  {
    timeClient.setTimeOffset(7200);
    sunTime.setTZOffset(2);
    summerTime = true;
  }
  sunTime.setPosition(LATITUDE, LONGITUDE, TIMEZONE);
  sunTime.setCurrentDate(currentYear, currentMonth, currentDay);
  sunriseCivilTime = (time_t)sunTime.calcCivilSunrise();
  sunsetCivilTime = (time_t)sunTime.calcCivilSunset();
  lightOn = false;
  startSecond = (minute(sunsetCivilTime)*3600)+(second(sunsetCivilTime)*60);
  #ifdef __SUNSET_DEBUG
    Serial.print("LATITUDE: ");
    Serial.println(LATITUDE);
    Serial.print("LONGITUDE: ");
    Serial.println(LONGITUDE);
    Serial.print("TIMEZONE: ");
    Serial.println(TIMEZONE);
    Serial.print("CURRENT TIME (h:m:s)- ");
    Serial.println(timeClient.getFormattedTime());
    Serial.print("SUNSET CIVIL TIME (h:m) - ");
    Serial.print(minute(sunsetCivilTime));
    Serial.print(":");
    Serial.println(second(sunsetCivilTime));
    Serial.print("SUNRISE CIVIL TIME (h:m) - ");
    Serial.print(minute(sunriseCivilTime));
    Serial.print(":");
    Serial.println(second(sunriseCivilTime));
    Serial.print("CURRENT DATE (d/m/y): ");
    Serial.print(currentDay);
    Serial.print("/");
    Serial.print(currentMonth);
    Serial.print("/");
    Serial.println(currentYear);
    Serial.print("SUMMERTIME BOOLEAN: ");
    Serial.println(summerTime);
    Serial.print("LIGHTON BOOLEAN: ");
    Serial.println(lightOn);
  #endif
  delay(WAITTIME*WTRETRY);
}




void loop() 
{
  Serial.print("Time  ");
  Serial.println(timeClient.getFormattedTime());
  actSecond = (timeClient.getHours()*3600)+(timeClient.getMinutes()*60);
  Serial.print("startSecond: ");
  Serial.print(startSecond);
  Serial.print(" * ActSecond: ");
  Serial.println(actSecond);
  if ( lightOn == false )
  {
    if ( actSecond >= startSecond )
    {
      digitalWrite(RELAYPIN, HIGH);
      lightOn = true;      
    }
  }
  else
  {
    if ( actSecond < startSecond )
    {
      // OFF LIGHT
      digitalWrite(RELAYPIN, LOW);
      delay(WAITTIME*WTRETRY);
      ESP.reset();
    }
  }
  Serial.print("LightOn: ");
  Serial.print(lightOn);
  Serial.print(" * ArduinoSecond: ");
  Serial.println(millis()/1000);
  Serial.print("NewDate: ");
  Serial.println(newDate);

  if ( (millis()/1000) > 10800 )
  {
    if ( newDate == false )
    {
      if ( timeClient.update() )
      {
        epochTime = timeClient.getEpochTime();
        ptm = gmtime ((time_t *)&epochTime); 
        currentDay = ptm->tm_mday;
        currentMonth = ptm->tm_mon+1;
        currentYear = ptm->tm_year+1900;
        sunTime.setCurrentDate(currentYear, currentMonth, currentDay);
        sunriseCivilTime = (time_t)sunTime.calcCivilSunrise();
        sunsetCivilTime = (time_t)sunTime.calcCivilSunset();
        startSecond = (minute(sunsetCivilTime)*3600)+(second(sunsetCivilTime)*60);
        newDate = true;
      }
      else
      {
        while ( !testForConnect() )
        {
          #ifdef __SUNSET_DEBUG
            Serial.println("..A MOMENT FOR NEW ATTEMPTS!\n");
          #endif
          delay(WAITTIME*WTRETRY);
        }
        while ( !testForNTP() )
        {
          #ifdef __SUNSET_DEBUG
            Serial.println("..A MOMENT FOR NEW ATTEMPTS!\n");
          #endif
          delay(WAITTIME*WTRETRY);
        }
      }
    }
  }
  
  delay(1000);
}
