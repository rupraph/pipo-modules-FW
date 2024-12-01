#pragma once
#ifndef PIPOCAPTIVE_H
#define PIPOCAPTIVE_H

#include <DNSServer.h>
#include <WiFi.h>

class CaptivePortal {
 public:
  WiFiUDP udp;

  CaptivePortal();
  bool is_active();
  void start();
  void stop();
  void loop();

 protected:
  bool active = false;
  DNSServer* dns_server = nullptr;
};
extern CaptivePortal captivePortal;

#endif