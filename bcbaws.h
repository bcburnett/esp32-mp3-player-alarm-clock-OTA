#ifndef BCBAWS
#define BCBAWS
#include <Arduino.h>
#include "bcbmp3.h"
#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <ESPAsyncWebServer.h>
#include <SD.h>

String getJson();
String File_Upload();
// web server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool wifiavail = false;


void notifyClients() { ws.textAll(getJson()); }

void notifySensorClients(const int msg) { ws.textAll(String(msg)); }

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    data[len] = 0;
    mp3Command((char *)data);
    notifyClients();
  }
}



void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    notifyClients();
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initWebServer() {
  // configure the web server routes

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->send(SD, "/index.htm", "text/html");});
  // start the web server
  server.begin();
}


String getJson() {
  StaticJsonDocument<250> data;
// struct mp3Status {
//   String playState;
//   String playMode;
//   int volume;
//   int currentTrack;
//   bool update;
//   int alarmHour;
//   int alarmMinute;
//   bool alarmSet;
// };

  data["playState"] = state.playState;
  data["playMode"] = state.playMode;
  data["volume"] = state.volume;
data["track"] =   state.currentTrack;
data["hour"] = state.alarmHour;
data["minute"] = state.alarmMinute;
data["set"] = state.alarmSet;
data["eq"] = state.eq;
  String response;
  serializeJson(data, response);
  return response;
}

#endif
