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
  bool isRunning();
  bool shouldStart();
  bool canStart();

 private:
  bool ws_initialized = false;
  bool should_start = false;
  bool is_running = false;
  unsigned long stopDate = 0;
  AsyncWebServer server;
  AsyncWebSocket ws;
  string received_configData;
};

/**
 * @brief Check if the WiFi is connected and the server is running
 */
bool pipoNetworkReady();

extern PipoServer server;

#endif  // WEBSERVER_H