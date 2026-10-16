#pragma once
#include <Arduino.h>
#include "wifi/wifi_policy.h"

/**
 * @brief WiFi battery-saving mode. Decides when the AP is needed and switches
 * between APSTA, STA only (modem sleep) and radio off.
 *
 * Only tick() changes the WiFi mode and it must run from wifiTask. Other
 * contexts (HTTP, WebSocket, buttons, WiFi events) only update timestamps or
 * flags.
 */
class PipoWifiPower {
 public:
  void begin();
  void tick();

  // Safe from any context
  void notifyActivity() { lastActivityTs = millis(); }
  void requestAP() { apRequested = true; }
  void onStaUp();
  void onStaDown();

  WifiTarget getCurrent() { return current; }

 private:
  static const uint32_t MIN_DWELL_MS = 3000;

  void apply(WifiTarget target);
  void readConfig(WifiPolicyIn& in);

  WifiTarget current = WifiTarget::APSTA;
  uint32_t lastTransitionTs = 0;
  uint32_t bootTs = 0;
  uint32_t apOpenTs = 0;
  volatile uint32_t lastActivityTs = 0;
  volatile uint32_t staUpSinceTs = 0;
  volatile uint32_t staLostSinceTs = 0;
  volatile bool apRequested = false;
};

extern PipoWifiPower wifiPower;
