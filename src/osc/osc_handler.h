#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "utils/config.h"
#include "utils/debug.h"

#ifdef PIPO_ANALOG
#include "sensors/analog_out.h"
#endif

class Config;

void oscreceiveTask(void* pvParameters);

class OSC_handler {
 public:
  // OSC_handler();

  void init();   // Initialize mutex and thread-safe resources
  void setup();  // Configure OSC settings
  void set_config();
  void send_osc_message(string address, float value);
  void send_battery_level(float voltage);
  void add_to_bundle(string address, float value);
  void bundle_has_messages();
  void send_bundle();

  void set_enabled(bool ena);
  bool is_enabled();
  bool is_started();
  void receive();

  void ensure_started();
  void stop();

 private:
  IPAddress dest_ip;
  int out_port;
  int localPort = 8001;  // Local port for receiving OSC
  bool isStarted = false;
  bool enabled = false;
  unsigned long lastConnectionTime = 0;  // Track when network connected

  OSCBundle bundle;

  WiFiUDP Udp;

  SemaphoreHandle_t mutex;  // Mutex for thread safety

  void set_dest_ip(string ip);
  void set_out_port(int port);
};

extern OSC_handler osc;

#endif  // OSC_HANDLER_H