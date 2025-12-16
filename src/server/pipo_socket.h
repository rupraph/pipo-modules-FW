#ifndef PIPOSOCKET_H
#define PIPOSOCKET_H
#include "sensors/sensors.h"
#include "utils/logs.h"
#include "utils/config.h"
#include "wifi/pipowifi.h"
#include "engine.h"
#define SO_REUSE 1
#define SO_REUSE_RXTOALL 1  // Enable reuse of addresses for multicast as well
#include <ESPAsyncWebServer.h>
#include "esp_task_wdt.h"
#include "server.h"
#include <set>
using namespace std;

void websocketTask(void* pvParameters);
class PipoSocket {
 public:
  PipoSocket();
  void setup();
  void sendNoteOn(int note, int velocity, int channel);
  void sendNoteOff(int note, int velocity, int channel);
  void sendSensorValue(std::string axis, float value);
  void loop();
  void stop();
  void start(AsyncWebSocket* ws);
  void pause();
  void resume();

 private:
  void onMessage(AsyncWebSocketClient* client);
  void cleanupDeadClients();
  void clearAllClients();
  void enforceOneClient(AsyncWebSocketClient* newClient);
  bool shouldAcceptConnection(AsyncWebSocketClient* newClient);
  AsyncWebSocket* ws;
  std::set<uint32_t>
      closingClients;  // Track clients we've already asked to close
  unsigned long lastSendTime = 0;
  unsigned long lastCleanTime = 0;
  unsigned long CLEANUP_INTERVAL =
      1000;  // Cleanup every 1 second instead of every loop
  unsigned long lastPingTime = 0;
  unsigned long lastConnectionTime = 0;
  IPAddress lastClientIP = IPAddress(0, 0, 0, 0);
  unsigned long MIN_CONNECTION_INTERVAL =
      500;  // Minimum 500ms between connections
  unsigned long PING_INTERVAL = 1000;
  unsigned long iterations = 0;
  const int inMaxLen = 512;
  int inMsgL = 0;
  char inMsg[512];
  const int outMaxLen = 256;
  int outMsgL = 0;
  char outMsg[256];

  bool paused = false;
};
extern PipoSocket pipoSocket;
#endif  // PIPOSOCKET_H