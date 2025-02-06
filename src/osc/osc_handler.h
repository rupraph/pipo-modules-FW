#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "utils/config.h"
// #include "sensors/analog_out.h"
#include "utils/debug.h"

class Config;

class OSC_handler {
 public:
  // OSC_handler();

  void setup();
  void set_config();
  void send_osc_message(string address, float value);

  void set_enabled(bool ena);
  bool get_enabled();
  void receive();

 private:
  IPAddress dest_ip;
  int out_port;
  bool isStarted = false;
  bool enabled = false;
  WiFiUDP Udp;
  void start();
  void stop();

  void set_dest_ip(string ip);
  void set_out_port(int port);
};

extern OSC_handler osc;

#endif  // OSC_HANDLER_H