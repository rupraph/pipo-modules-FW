#ifndef PIPOWIFI_H
#define PIPOWIFI_H
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <iostream>
#include <map>
#include <string>
class PipoWifi {

public:
  enum PipoWifiMode { CONNECTING, AP, CONNECTED };
  PipoWifi();
  void setup();
  bool is_running = false;
  PipoWifiMode getMode();
  std::string status();
  std::string availableNetworks();
  bool connect(std::string ssid, std::string password);
  void scan();

private:
  static const uint CONNECT_TIMEOUT = 2000;
  static const uint CHECK_TIMEOUT = 50;
  PipoWifiMode mode = CONNECTING;
  Preferences preferences;
  std::map<std::string, float> ssids;
  void connect();
  void save(std::string ssid, std::string password);
  void APMode();
};

#endif // PIPOWIFI_H
