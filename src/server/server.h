#ifndef PIPOSERVER_H
#define PIPOSERVER_H
#include "HW_CONFIG.h"
#include "engine.h"
#include "server/midi_socket.h"
#include "utils/config.h"
#include "utils/fs_tools.h"
#include "utils/json.hpp"
#include "utils/logs.h"
#include "wifi/wifi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

using namespace std;

class PipoServer {
public:
  PipoServer(Sensor &sensor, Engine &engine, OSC_handler &osc, PipoWifi &wm)
      : server(80), ws("/ws"), events("/events"), input_sens(sensor),
        engine(engine), osc(osc), wm(wm) {}
  void setup();
  void setup_ws();
  void onMessage(AsyncWebSocketClient *client, String message);
  void setup_requests();
  void stop(); // Todo: start stop not much tested yet
  bool is_running = false;

private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  AsyncEventSource events;
  Sensor &input_sens;
  Engine &engine;
  OSC_handler &osc;
  PipoWifi &wm;
  string received_configData;
  // json parsed_configData;
};
#endif // WEBSERVER_H