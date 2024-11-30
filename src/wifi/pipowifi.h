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
  /**
  * @brief The current status of the wifi
  */
  enum PipoWifiStatus { CONNECTING, CONNECTED, DISCONNECTED };
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
  bool connect(String ssid, String password, bool disconnect = true);
  /**
   * @brief Scans for available networks
   */
  void scan();
  static const uint CONNECT_TIMEOUT = 2000;
  static const uint CHECK_TIMEOUT = 50;
  PipoWifiStatus status = DISCONNECTED;
  PipoPWManager pwm;
  std::map<String, int> signals;
  /**
   * @brief Tries to connect to a WIFI network, fallback to AP if it fails
   * @return true if successfully connected to a WIFI network, false otherwise
   */
  bool connect();
  /**
   * @brief Switches to AP mode
   * @return true if AP mode is successfully set, false otherwise
   */
  bool APMode();
  /**
   * @brief Tries to switch to AP mode, fallback to AP if it fails
   * @return true if successfully connected to a WIFI network, false otherwise
   */
  bool APSTAMode();
  /**
   * @brief Tries to switch to STA mode, fallback to AP if it fails
   * @return true if successfully connected to a WIFI network, false otherwise
   */
  bool STAMode();
};

extern PipoWifi wifi;

#endif  // PIPOWIFI_H
