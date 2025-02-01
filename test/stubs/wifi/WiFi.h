#ifndef PIPOSTUBWIFI_H
#define PIPOSTUBWIFI_H
#include <Arduino.h>

#define WIFI_OFF 1
#define WIFI_STA 2
#define WIFI_AP 3
#define WIFI_AP_STA 4
#define WIFI_MODE_APSTA 5
#define WL_CONNECTED 6

class IPAddressStub {
 public:
  String toString();
};

class WiFiStub {
 public:
  int begin(String ssid, String password);
  void softAP(String ssid, String password);
  int status();
  void disconnect(bool force = false);
  void mode(int mode);
  void setAutoReconnect(bool b);
  int scanNetworks(bool a, bool b, bool c, uint d);
  int SSID(int i);
  String SSID();
  IPAddressStub localIP();
  int RSSI(int i);
  String getString(String key, String defaultValue);
};

WiFiStub WiFi;

#endif  // PIPOSTUBWIFI_H