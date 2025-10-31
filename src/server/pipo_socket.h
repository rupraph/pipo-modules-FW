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
#include "server.h"
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
  AsyncWebSocket* ws;
  unsigned long lastSendTime = 0;
  unsigned long lastCleanTime = 0;
  unsigned long lastPingTime = 0;
  unsigned long PING_INTERVAL = 1000;
  unsigned long iterations = 0;
  const int inMaxLen = 2048;
  int inMsgL = 0;
  char inMsg[2048];
  const int outMaxLen = 256;
  int outMsgL = 0;
  char outMsg[256];

  bool toSend[1] = {true};
  bool paused = false;
};
extern PipoSocket pipoSocket;
#endif  // PIPOSOCKET_H