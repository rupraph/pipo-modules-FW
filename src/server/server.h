#ifndef PIPOSERVER_H
#define PIPOSERVER_H
#include <ESPmDNS.h>
#include <vector>
#include "HW_CONFIG.h"
#include "engine.h"
#include <ArduinoJson.h>
#include "utils/config.h"
#include "utils/logs.h"
#include "utils/fs_tools.h"
#include "utils/debug.h"
#include "server/pipo_socket.h"
#include "server/captive-portal.h"
#include "server/presets.hpp"
#include "wifi/pipowifi.h"
#include "HW_CONFIG.h"

#define stringify(s) _stringifyDo(s)
#define _stringifyDo(s) #s

using namespace std;

class PipoServer {
 public:
  PipoServer() : server(80), ws("/ws") {}
  void setup();
  void setup_requests();
  void pause();
  void resume();
  bool isRunning();

 private:
  bool is_running = false;
  AsyncWebServer server;
  AsyncWebSocket ws;
  PipoPresets presets;
  string received_configData;
};

/**
 * @brief Check if the WiFi is connected and the server is running
 */
bool pipoNetworkReady();

extern PipoServer server;

#endif  // WEBSERVER_H