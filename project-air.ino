// Board:             DOIT ESP32 (Node ESP?)
// Documentation URL: asdasd
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

// WIFI
const char* ssid = "AIRWIFI";
const char* password = "12345678";
String serverName = "http://martorenzo.click/project/air/server/";  // include "/"
String deviceName = "1";                 
int wifiConnected = 0;                          
int wifiStatus = 2; // 0 - manual
                    // 1 - wifi
                    // 2 - choose

// DISPLAY
LiquidCrystal_I2C lcd(0x27, 16, 2); 
int menuDisplay = 0;                  // 0 - manual > main
                                      // 1 - manual > fan
                                      // 2 - manual > spray

// AIR
int sensorAir = 33;

// Control - Out
int pinfan1 = 25;
int pinfan2 = 26;
int pinfan3 = 27;
int pinSpray = 13;
int pinLedGreen = 15;
int pinLedRed = 4;
int pinLedOrange = 2;

// Button
int btnFan = 19;                         
int btnSpray = 18;                      

// Data
String statusAir = "0";                       // 0 - green  good        0-5%
                                              // 1 - orange not good    6-50%
                                              // 2 - red    pulluted    51-99%
String statusAirValue = "";                   // %
String statusAirValue2 = "";                  // GOOD
                                              // CLEANING
                                              // BAD

String statusFan = "0";                       // 0 - off
                                              // 1 - level1
                                              // 2 - level2
                                              // 3 - level3
String statusFanValue = "";
String statusFanSettingValue = "";

String statusSpray2 = "0"; 
String statusSprayLast = "-1";   
String statusSpray = "0";                     // 0 - off
                                              // 1 - 1 min
                                              // 2 - 3 min
                                              // 3 - 5 min
                                              // 4 - 10 min
                                              // 5 - 15 min
String statusSprayValue = "";
String statusSpraySettingValue = "";
String statusCalibratedAirMin = "0";
String statusCalibratedAirMax = "1023";

// debounce
unsigned long lastTime = 0;                   // wifi / data
unsigned long lastTimeAir = 0;
unsigned long lastTimeFan = 0;
unsigned long lastTimeSpray = 0;            // 
unsigned long lastTimeSprayDeb = 0;         // deb
unsigned long lastTimeDisplay = 0;
unsigned long lastTimeBtn = 0;    

unsigned long timerDelay1 = 1000;
unsigned long timerDelay2 = 500;
unsigned long timerDelaySpray = 0;           // countdown
unsigned long timerDelaySprayOrig = 0;        // countdown max
unsigned long timerDelayDisplay = 5000;      // manual / setting (fan/spray)


// ============================
// Start
// ============================
void setup() 
{
  // Serial
  Serial.begin(9600);

  // init lcd
  lcd.init();
  lcd.backlight(); 
  lcd.clear();

  // init Button
  pinMode(sensorAir, INPUT);

  // init Control - Out
  pinMode(pinfan1, OUTPUT);
  pinMode(pinfan2, OUTPUT);
  pinMode(pinfan3, OUTPUT);
  pinMode(pinSpray, OUTPUT);
  pinMode(pinLedGreen, OUTPUT);
  pinMode(pinLedRed, OUTPUT);
  pinMode(pinLedOrange, OUTPUT);
  digitalWrite(pinfan1, LOW);
  digitalWrite(pinfan2, LOW);
  digitalWrite(pinfan3, LOW);
  digitalWrite(pinSpray, LOW);
  digitalWrite(pinLedGreen, LOW);
  digitalWrite(pinLedRed, LOW);
  digitalWrite(pinLedOrange, LOW);
  
  // init Button
  pinMode(btnFan, INPUT_PULLUP);
  pinMode(btnSpray, INPUT_PULLUP);
}


// ============================
// Loop
// ============================
void loop() 
{
  // Get Air
  int sensorAirVal = analogRead(sensorAir); 
  sensorAirVal = map(sensorAirVal, statusCalibratedAirMin.toInt(), statusCalibratedAirMax.toInt(), 0, 100);
  statusAirValue = String(sensorAirVal) + "%";
  statusAir = String(sensorAirVal);

  // Button
  int btnFanVal = digitalRead(btnFan);
  int btnSprayVal = digitalRead(btnSpray);

  // AUTO
  if (wifiStatus == 1)
  {
    //
    if (wifiConnected == 0)
    {
      ConnectWifi();
    }

    //
    if ((millis() - lastTime) > timerDelay1) 
    {
      // set
      RequestSetData();

      // get
      RequestGetFan();
      RequestGetSpray();
      RequestGetSpray2();

      // Connected
      if(WiFi.status()== WL_CONNECTED)
      {
        lcd.setCursor(0, 0);        
        lcd.print("AQ:     " + statusAirValue2);
        lcd.setCursor(0, 1);        
        lcd.print("Fan " + statusFanValue + "    SPR " + statusSprayValue);
      }

      // Disconnected
      else 
      {
          lcd.setCursor(0, 0);        
          lcd.print("   CONNECTING   "); 
          lcd.setCursor(0, 1);        
          lcd.print("      WIFI      "); 
      }

      // Next
      lastTime = millis();
    }
  }

  // MANUAL
  if (wifiStatus == 0)
  {
    // display back?
    if ((millis() - lastTimeDisplay) > timerDelayDisplay) 
    {
      menuDisplay = 0;
      lastTimeDisplay = millis();
    }

    // button?
    if ((millis() - lastTimeBtn) > timerDelay2) 
    {
      // fan?
      if (!btnFanVal)
      {
        lastTimeBtn = millis();
        lastTimeDisplay = millis();

        if (menuDisplay == 1)
        {
          if (statusFan == "0")
          {
            statusFan = "1";
            return;
          }

          if (statusFan == "1")
          {
            statusFan = "2";
            return;
          }

          if (statusFan == "2")
          {
            statusFan = "3";
            return;
          }

          if (statusFan == "3")
          {
            statusFan = "0";
            return;
          }
        }

        menuDisplay = 1;
      }

      // spray?
      if (!btnSprayVal)
      {
        lastTimeBtn = millis();
        lastTimeDisplay = millis();

        if (menuDisplay == 2)
        {
          if (statusSpray == "0")
          {
            statusSpray = "1";
            return;
          }

          if (statusSpray == "1")
          {
            statusSpray = "2";
            return;
          }

          if (statusSpray == "2")
          {
            statusSpray = "3";
            return;
          }

          if (statusSpray == "3")
          {
            statusSpray = "4";
            return;
          }

          if (statusSpray == "4")
          {
            statusSpray = "5";
            return;
          }

          if (statusSpray == "5")
          {
            statusSpray = "0";
            return;
          }
        }

        menuDisplay = 2;
      }
    }

    // Main Display
    if (menuDisplay == 0)
    {
      lcd.setCursor(0, 0);        
      lcd.print("AQ:     " + statusAirValue2);
      lcd.setCursor(0, 1);        
      lcd.print("Fan " + statusFanValue + "    SPR " + statusSprayValue);
      //lcd.print("                "); 
    }

    // Fan Display
    if (menuDisplay == 1)
    {
      lcd.setCursor(0, 0);        
      lcd.print("   FAN  LEVEL   ");
      lcd.setCursor(0, 1);    
      lcd.print(statusFanSettingValue);
      //lcd.print("                "); 
    }

    // Spray Display
    if (menuDisplay == 2)
    {
      lcd.setCursor(0, 0);        
      lcd.print(" SPRAY INTERVAL ");
      lcd.setCursor(0, 1);        
      lcd.print(statusSpraySettingValue);
      //lcd.print("                "); 
    }
  }

  // CHOOSE
  if (wifiStatus == 2)
  {
    lcd.setCursor(0, 0);        
    lcd.print("  CHOOSE  MODE  "); 
    lcd.setCursor(0, 1);        
    lcd.print("1-Auto  2-Manual");
    //lcd.print("                "); 

    // Btn?
    if ((millis() - lastTimeBtn) > timerDelay1) 
    {
      if (!btnFanVal)
      {
        wifiStatus = 1;
        Serial.println("aaa");

        // Next
        lastTimeBtn = millis();
        return;
      }

      if (!btnSprayVal)
      {
        wifiStatus = 0;
        Serial.println("bbb");

        // Next
        lastTimeBtn = millis();
        return;
      }
    }
  }

  // Air?
  {
    ConvertAir(sensorAirVal);
    if ((millis() - lastTimeAir) > timerDelay1) 
    {
      // check status
      if (wifiStatus == 2)
      {
        return;
      }

      if (sensorAirVal >= 0 && sensorAirVal <= 5)
      {
        digitalWrite(pinLedGreen, HIGH);
        digitalWrite(pinLedRed, LOW);
        digitalWrite(pinLedOrange, LOW);
      }

      if (sensorAirVal >= 6 && sensorAirVal <= 50)
      {
        digitalWrite(pinLedGreen, LOW);
        digitalWrite(pinLedRed, LOW);
        digitalWrite(pinLedOrange, HIGH);
      }

      if (sensorAirVal >= 51)
      {
        digitalWrite(pinLedGreen, LOW);
        digitalWrite(pinLedRed, HIGH);
        digitalWrite(pinLedOrange, LOW);
      }

      lastTimeAir = millis();
    }
  }

  // Fan?
  {
    ConvertFan();
    if ((millis() - lastTimeFan) > timerDelay1) 
    {
      // check status
      if (wifiStatus == 2)
      {
        return;
      }

      if (statusFan == "0")
      {
        digitalWrite(pinfan1, LOW);
        digitalWrite(pinfan2, LOW);
        digitalWrite(pinfan3, LOW);
      }

      if (statusFan == "1")
      {
        digitalWrite(pinfan1, HIGH);
        digitalWrite(pinfan2, LOW);
        digitalWrite(pinfan3, LOW);
      }

      if (statusFan == "2")
      {
        digitalWrite(pinfan1, LOW);
        digitalWrite(pinfan2, HIGH);
        digitalWrite(pinfan3, LOW);
      }

      if (statusFan == "3")
      {
        digitalWrite(pinfan1, LOW);
        digitalWrite(pinfan2, LOW);
        digitalWrite(pinfan3, HIGH);
      }

      lastTimeFan = millis();
    }
  }

  // Spray?
  {
    ConvertSpray();
    /*
    if ((millis() - lastTimeSprayDeb) > timerDelay1) 
    {
      // check status
      if (wifiStatus == 2)
      {
        return;
      }

      // check ON
      if (timerDelaySpray > 0)
      {
        digitalWrite(pinSpray, HIGH);
        lastTimeSpray = millis();
      }
    }
    else
    {
      digitalWrite(pinSpray, LOW);
    }
    */

    if ((millis() - lastTimeSprayDeb) > timerDelaySpray * 1000) 
    {
      // check status
      if (wifiStatus == 2)
      {
        return;
      }
      
      lastTimeSprayDeb = millis();

      digitalWrite(pinSpray, HIGH);
      delay(1000);
      digitalWrite(pinSpray, LOW);
    }
  }

  // Manual Spray
  if (statusSpray2 == "1")
  {
    digitalWrite(pinSpray, HIGH);
    delay(1000);
    digitalWrite(pinSpray, LOW);
    statusSpray2 = "0";
  }
}


void ConnectWifi()
{
  lcd.setCursor(0, 0);        
  lcd.print("   CONNECTING   "); 
  lcd.setCursor(0, 1);        
  lcd.print("      WIFI      "); 

  Serial.println("init wifi");
  // init wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  wifiConnected = 1;
}

// Get Fan
void RequestGetFan()
{
  HTTPClient http;
  String serverPath = serverName + "api.php?mode=getfan&id=" + deviceName;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    String payload = http.getString();
    Serial.println(payload);
    statusFan = payload;
  }

  http.end();
}

// Get Spray
void RequestGetSpray()
{
  HTTPClient http;
  String serverPath = serverName + "api.php?mode=getspray&id=" + deviceName;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    String payload = http.getString();
    Serial.println(payload);
    statusSpray = payload;
  }

  http.end();
}

// Get Spray
void RequestGetSpray2()
{
  HTTPClient http;
  String serverPath = serverName + "api.php?mode=getspray2&id=" + deviceName;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    String payload = http.getString();
    Serial.println(payload);
    statusSpray2 = payload;
  }

  http.end();
}

// Set Data
void RequestSetData()
{
  HTTPClient http;
  String serverPath = serverName + "api.php?mode=setdata&id=" + deviceName + "&val1=" + statusAir;
  http.begin(serverPath.c_str());
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    String payload = http.getString();
    Serial.println(payload + " " + statusAir);
  }

  http.end();
}

void ConvertAir(long sensorAirVal)
{
  if (sensorAirVal >= 0 && sensorAirVal <= 5)
  {
    statusAirValue2 = "    GOOD";
  }

  if (sensorAirVal >= 6 && sensorAirVal <= 50)
  {
    statusAirValue2 = "CLEANING";
  }

  if (sensorAirVal >= 51)
  {
    statusAirValue2 = "     BAD";
  }
}

void ConvertFan()
{
  if (statusFan == "0")
  {
    statusFanValue = "OF";
    statusFanSettingValue = "    TURN OFF    ";
  }

  if (statusFan == "1")
  {
    statusFanValue = "1 ";
    statusFanSettingValue = "      LOW       ";
  }

  if (statusFan == "2")
  {
    statusFanValue = "2 ";
    statusFanSettingValue = "     MEDIUM     ";
  }

  if (statusFan == "3")
  {
    statusFanValue = "3 ";
    statusFanSettingValue = "      HIGH      ";
  }
}

void ConvertSpray()
{
  // Check Set
  if (statusSprayLast == statusSpray)
  {
    return;
  }

  // Set New
  if (statusSpray == "0")
  {
    timerDelaySpray = 0;
    timerDelaySprayOrig = 0;
    statusSprayLast = statusSpray;
    statusSprayValue = "OF";
    statusSpraySettingValue = "    TURN OFF    ";
  }

  if (statusSpray == "1")
  {
    timerDelaySpray = 60;
    timerDelaySprayOrig = 60;
    statusSprayLast = statusSpray;
    statusSprayValue = "01";
    statusSpraySettingValue = "    1 Minute    ";
  }

  if (statusSpray == "2")
  {
    timerDelaySpray = 180;
    timerDelaySprayOrig = 180;
    statusSprayLast = statusSpray;
    statusSprayValue = "03";
    statusSpraySettingValue = "    3 Minutes   ";
  }

  if (statusSpray == "3")
  {
    timerDelaySpray = 300;
    timerDelaySprayOrig = 300;
    statusSprayLast = statusSpray;
    statusSprayValue = "05";
    statusSpraySettingValue = "    5 Minutes   ";
  }

  if (statusSpray == "4")
  {
    timerDelaySpray = 600;
    timerDelaySprayOrig = 600;
    statusSprayLast = statusSpray;
    statusSprayValue = "10";
    statusSpraySettingValue = "   10 Minutes   ";
  }

  if (statusSpray == "5")
  {
    timerDelaySpray = 900;
    timerDelaySprayOrig = 900;
    statusSprayLast = statusSpray;
    statusSprayValue = "15";
    statusSpraySettingValue = "   15 Minutes   ";
  }
}
