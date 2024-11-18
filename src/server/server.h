#ifndef PIPOSERVER_H
#define PIPOSERVER_H
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "engine.h"
#include "utils/config.h"
#include <ArduinoJson.h>
#include "utils/logs.h"
#include "utils/fs_tools.h"
#include "server/pipo_socket.h"
#include "HW_CONFIG.h"

#define stringify(s) _stringifyDo(s)
#define _stringifyDo(s) #s

using namespace std;

class PipoServer {
 public:
  PipoServer(Sensor& sensor, Engine& engine, OSC_handler& osc)
      : server(80),
        ws("/ws"),
        events("/events"),
        input_sens(sensor),
        engine(engine),
        osc(osc) {}
  void setup();
  void setup_ws();
  void onMessage(AsyncWebSocketClient* client);
  void setup_requests();
  void stop();  // Todo: start stop not much tested yet
  bool is_running = false;

 private:
  AsyncWebServer server;
  AsyncWebSocket ws;
  AsyncEventSource events;
  Sensor& input_sens;
  Engine& engine;
  OSC_handler& osc;
  string received_configData;
  const int ws_max_len = 2048;
  int ws_message_len = 0;
  char ws_message[2048];
};
#endif  // WEBSERVER_H