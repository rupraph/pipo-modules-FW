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
  String status();
  String availableNetworks();
  bool connect(String ssid);
  bool connect(String ssid, String password);
  void scan();
  void decode(char);
  void load();

  static const int MAX_NETWORKS = 10;
  static const uint CONNECT_TIMEOUT = 2000;
  static const uint CHECK_TIMEOUT = 50;
  PipoWifiMode mode = CONNECTING;
  Preferences preferences;
  std::map<String, int> signals;
  std::map<String, String> passwords;
  void connect();
  void save();
  void APMode();
};

#endif  // PIPOWIFI_H
