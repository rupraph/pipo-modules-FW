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
#include "server/captive-portal.h"
#include "wifi/pipowifi.h"
#include "HW_CONFIG.h"

#define stringify(s) _stringifyDo(s)
#define _stringifyDo(s) #s

using namespace std;

class PipoServer {
 public:
  PipoServer() : server(80), ws("/ws") {}
  void setup();
  void setup_ws();
  void setup_requests();
  void start();
  void stop();
  bool is_running = false;
  bool should_start = false;
  bool ws_initialized = false;

 private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  string received_configData;
};

extern PipoServer server;

#endif  // WEBSERVER_H