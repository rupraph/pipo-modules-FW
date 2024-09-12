#ifndef PIPOSOCKET_H
#define PIPOSOCKET_H
#include <ESPAsyncWebServer.h>
#include "sensor/input_sensor.h"
#include "utils/logs.h"
using namespace std;

class PipoSocket {
 public:
  PipoSocket();
  void setup(AsyncWebSocket* ws, Sensor* sensor);
  void sendNoteOn(int note, int velocity, int channel);
  void sendNoteOff(int note, int velocity, int channel);
  void sendSensorValue(std::string axis, float value);
  void loop();

 private:
  AsyncWebSocket* ws;
  Sensor* input_sens;
  unsigned long lastSendTime = 0;
  unsigned long iterations = 0;
};
extern PipoSocket pipoSocket;
#endif  // PIPOSOCKET_H