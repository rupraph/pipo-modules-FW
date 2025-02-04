#pragma once
#ifndef PIPOCAPTIVE_H
#define PIPOCAPTIVE_H

#include <DNSServer.h>
#include <WiFi.h>
#include <wifi/pipowifi.h>
#include <ESPAsyncWebServer.h>

class CaptivePortal {
 public:
  CaptivePortal();
  bool is_active();
  bool canEnable();
  void start(AsyncWebServer* server);
  void stop();
  void loop();

 protected:
  DNSServer* dns_server = nullptr;
};
extern CaptivePortal captivePortal;

#endif