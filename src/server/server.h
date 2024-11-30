#ifndef PIPOSERVER_H
#define PIPOSERVER_H
#include <ESPmDNS.h>
#include "HW_CONFIG.h"
#include "engine.h"
#include <ArduinoJson.h>
#include "utils/config.h"
#include "utils/logs.h"
#include "utils/fs_tools.h"
#include "utils/debug.h"
#include "server/pipo_socket.h"
#include "wifi/pipowifi.h"
#include "HW_CONFIG.h"

#define stringify(s) _stringifyDo(s)
#define _stringifyDo(s) #s

using namespace std;

class PipoServer {
 public:
  PipoServer() : server(80), ws("/ws"), events("/events") {}
  void setup();
  void setup_ws();
  void onMessage(AsyncWebSocketClient* client);
  void setup_requests();
  void start();
  void stop();
  bool is_running = false;
  bool should_start = false;
  bool ws_initialized = false;

 private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  AsyncEventSource events;

  string received_configData;
  const int ws_max_len = 2048;
  int ws_message_len = 0;
  char ws_message[2048];
};

extern PipoServer server;

#endif  // WEBSERVER_H