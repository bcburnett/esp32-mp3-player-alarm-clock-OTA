#include <Arduino.h>
#include "bcbaws.h"
#include "bcbmp3.h"
#include "bcbsdcard.h"
#include "time.h"
#include "WiFiCred.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define ARDUINO_RUNNING_CORE 1

// internal rtc
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;



U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);



// localtime from internal rtc
String printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return ("");
  }
  return (asctime(&timeinfo));
}

String printLocalHour() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return ("");
  }
  return (String(asctime(&timeinfo)).substring(11, 16) + " ");
}

int getHour() {
  time_t now = time(NULL);
  struct tm *tm_struct = localtime(&now);
  int hour = tm_struct->tm_hour;
  return hour;
}

int getMinute() {
  time_t now = time(NULL);
  struct tm *tm_struct = localtime(&now);
  int minute = tm_struct->tm_min;
  return minute;
}

// define functions
void TaskRelay(void *pvParameters);
void TaskAlarm(void *pvParameters);
void initWiFi();
void initTime();

void setup() {
  // start the serial interface
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(3000);
  initWiFi();
  initTime();
  initWebServer();
  initWebSocket();
  u8g2.begin();
  initSDCard();
  mp3Begin();

  xTaskCreatePinnedToCore(TaskRelay, "TaskRelay" // A name just for humans
                          ,
                          4096 // This stack size can be checked & adjusted by
                          // reading the Stack Highwater
                          ,
                          NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1)
                          // being the highest, and 0 being the lowest.
                          ,
                          NULL, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(TaskAlarm, "TaskAlarm" // A name just for humans
                          ,
                          4096 // This stack size can be checked & adjusted by
                          // reading the Stack Highwater
                          ,
                          NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1)
                          // being the highest, and 0 being the lowest.
                          ,
                          NULL, ARDUINO_RUNNING_CORE);

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
    // using SPIFFS.end()
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
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();
  if (WiFi.status() != WL_CONNECTED)
    initWiFi();
  vTaskDelay(60);
}

void TaskRelay(void *pvParameters) { // handle websocket and oled displays
  (void)pvParameters;
  for (;;) {
    u8g2.clearBuffer();              // clear the internal memory
    u8g2.setFont(u8g2_font_8x13_tf); // choose a suitable font
    
    u8g2.drawStr(
      0, 15,
      printLocalHour().c_str()); // write something to the internal memory
      
    u8g2.drawStr(
      64, 15,
      state.playState.c_str()); // write something to the internal memory
      
    u8g2.drawStr(
      0, 30,
      state.playState.c_str()); // write something to the internal memory
      
    u8g2.drawStr(75, 30,
                 String(state.currentTrack)
                 .c_str()); // write something to the internal memory
                 
    u8g2.drawStr(0, 45,
                 (String("Vol: ") + String(state.volume))
                 .c_str());     // write something to the internal memory
                 
    u8g2.drawStr(0, 60, (String("Alarm: ") + String(state.alarmHour) + ":" + String(state.alarmMinute)).c_str()); // write something to the internal memory
    u8g2.sendBuffer();              // transfer internal memory to the display
    setMP3Status();
    notifyClients();
    vTaskDelay(15000);
  }
}

void TaskAlarm(void *pvParameters) { // handle websocket and oled displays
  (void)pvParameters;
  for (;;) {
    if (state.alarmSet) {
      if (getHour() == state.alarmHour) {
        if (getMinute() == state.alarmMinute) {
          soundAlarm();
        }
      }
    }
    vTaskDelay(15000);
  }
}
void initWiFi() {
  // connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    return;
  }
  wifiavail = true;
}

void initTime() {
  // set the clock from ntp server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
