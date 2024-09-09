#ifndef PIPOWIFI_H
#define PIPOWIFI_H
#include <Preferences.h>
#include <WiFi.h>
#include <map>
#include <string>
class PipoWifi {

public:
  enum PipoWifiMode { CONNECTING, AP, CONNECTED };
  PipoWifi();
  void setup();
  bool is_running = false;
  PipoWifiMode getMode();

private:
  PipoWifiMode mode = CONNECTING;
  Preferences preferences;
  std::map<std::string, float> ssids;
  void scan();
  void connect();
  void save(std::string ssid, std::string password);
  bool connect(std::string ssid, std::string password);
  void APMode();
};

#endif // PIPOWIFI_H
