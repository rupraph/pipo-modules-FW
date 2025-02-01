#ifndef PIPOSOCKET_H
#define PIPOSOCKET_H
#include "sensors/sensors.h"
#include "utils/logs.h"
#include "utils/config.h"
#include "wifi/pipowifi.h"
#include "engine.h"
#include <ESPAsyncWebServer.h>
using namespace std;
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

 private:
  void onMessage(AsyncWebSocketClient* client);
  AsyncWebSocket* ws;
  unsigned long lastSendTime = 0;
  unsigned long lastCleanTime = 0;
  unsigned long iterations = 0;
  const int ws_max_len = 2048;
  int ws_message_len = 0;
  char ws_message[2048];
  bool toSend[1] = {true};
};
extern PipoSocket pipoSocket;
#endif  // PIPOSOCKET_H