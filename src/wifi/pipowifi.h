#ifndef PIPOWIFI_H
#define PIPOWIFI_H
#include <Arduino.h>
#include <Preferences.h>
#include <wifi/pw-manager.hpp>
#include <WiFi.h>
#include <iostream>
#include <map>
#include <string>

class PipoWifi {

 public:
  enum PipoWifiMode { CONNECTING, CONNECTED, DISCONNECTED };
  PipoWifi();
  void setup();
  bool is_running = false;
  PipoWifiMode getMode();
  String status();
  String ssid();
  String availableNetworks();
  bool connect(String ssid);
  bool connect(String ssid, String password);
  void scan();
  void decode(char);

  static const uint CONNECT_TIMEOUT = 2000;
  static const uint CHECK_TIMEOUT = 50;
  PipoWifiMode mode = DISCONNECTED;
  PipoPWManager pwm;
  std::map<String, int> signals;
  void connect();
  void APMode();
};

extern PipoWifi wifi;

#endif  // PIPOWIFI_H
