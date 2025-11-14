#ifndef PIPOWIFI_H
#define PIPOWIFI_H
#include <Arduino.h>
#include <Preferences.h>
#include <wifi/pw-manager.hpp>
#include <WiFi.h>
#include <iostream>
#include <map>
#include <string>
#include "utils/config.h"
#include "shared_flags.h"

// enum WiFiTransition {
//   NONE,
//   AP_TO_STA,
//   STA_TO_AP,
//   STA_TO_APSTA,
//   APSTA_TO_STA,
//   AP_TO_APSTA,
//   APSTA_TO_AP
// };

// Forward declarations for WiFi event handlers
void onWiFiReadyHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onScanDoneHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTAStartHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTAStopHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTAConnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTADisconnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTAAuthModeChangeHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTAGotIPHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTAGotIP6Handler(WiFiEvent_t event, WiFiEventInfo_t info);
void onSTALostIPHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStartHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStopHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStationConnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStationDisconnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPStationIPAssignedHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPProbeReqReceivedHandler(WiFiEvent_t event, WiFiEventInfo_t info);
void onAPGotIP6Handler(WiFiEvent_t event, WiFiEventInfo_t info);

void wifiTask(void* pvParameters);
struct PipoWState {
  // WiFiTransition transition = NONE;
  wifi_mode_t mode = WIFI_MODE_APSTA;
  String ssid = "";
  String password = "";
  bool shouldScan = false;
  bool shouldRSSI = false;
  bool shouldDisconnect = false;
};

class PipoWifi {
 private:
  // Private implementation details not needed by event handlers
  static const uint CONNECT_TIMEOUT = 10000;
  static const uint CHECK_TIMEOUT = 1000;
  const int WIFI_DELAY = 2000;
  const int MIN_SUBNET = 10;
  int subnetBase = MIN_SUBNET;
  IPAddress apIP = IPAddress(192, 168, subnetBase, 1);
  IPAddress apMask = IPAddress(255, 255, 255, 0);
  unsigned long lastScan = 0;
  bool shouldRefreshRSSI = true;

  void getFreeSubNet();

 public:
  // Public state accessible by event handlers - embedded systems pragmatic approach
  bool scanning = false;
  bool isChangingAP = false;  // means switching from one AP to another ?
  PipoWState next;
  PipoPWManager pwm;
  std::map<String, int> signals;
  int8_t rssi;

  /**
  * @brief The current status of the wifi
  */
  enum PipoWifiStatus { CONNECTING, CONNECTED, DISCONNECTED };
  PipoWifiStatus status = DISCONNECTED;

  // Public methods
  void saveScanResult();
  bool configureAP();
  PipoWifi();
  void setup();
  /**
   * @brief Returns the current status of the wifi
   * @return the current status of the wifi
   */
  PipoWifiStatus getStatus();
  /**
   * @brief Returns the current state of the wifi (mode, status, IP, ssid)
   */
  String state();
  /**
   * @brief Returns the current ssid
   */
  String ssid();
  /**
   * @brief Returns the available networks
   * @return a a line per network with ssid signal isConnected hasKnwonPassword
   */
  String availableNetworks();
  /**
   * @brief Tries to Connects to a WIFI network, fallback to AP if it fails
   * @param ssid the ssid of the network to connect to. Takes the remembered password if it exists
   * @return true if successfully connected to a WIFI network, false otherwise
   */
  bool connect(String ssid);
  /**
   * @brief Tries to Connects to a WIFI network, fallback to AP if it fails
   * @param ssid the ssid of the network to connect to
   * @param password the password of the network to connect to
   * @param disconnect if true, disconnects from the current network before connecting
   */
  bool connect(String ssid, String password);
  /**
   * @brief Tries to connect to a WIFI network, fallback to AP if it fails
   * @return true if successfully connected to a WIFI network, false otherwise
   */
  bool connect();
  /**
   * @brief Switches to AP mode
   * @return true if AP mode is successfully set, false otherwise
   */
  /*
  * @brief Returns true if a scan is currently running
  */
  bool isScanning();
  bool ready();
  int8_t getRSSI();
  void step();
  void refresh();
  void setMode(wifi_mode_t mode);
  void setSSID(String ssid);
  void setPassword(String password);
  void requestScan();
  void requestRSSI();
  /**
   * @brief Forgets a network by removing it from the password manager
   * @param ssid the ssid of the network to forget
   */
  void forgetNetwork(String ssid);
};

extern PipoWifi wifi;

#endif  // PIPOWIFI_H
