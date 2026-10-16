#pragma once
#include <stdint.h>

// Pure WiFi power-save policy: no Arduino / WiFi dependencies so it can be
// unit tested natively. All timestamps are millis(); 0 means "never".

enum class WifiTarget : uint8_t { APSTA = 0, STA_ONLY = 1, OFF = 2 };

// STA must hold an IP this long before the AP is allowed to drop
static const uint32_t WIFI_STA_STABLE_MS = 5000;

struct WifiPolicyIn {
  bool enabled = false;     // power-save mode enabled in config
  bool oscEnabled = false;  // OSC output enabled (AP is a data path)
  bool wsClient = false;    // a config page WebSocket client is connected
  bool apStation = false;   // a device is associated to the AP
  WifiTarget current = WifiTarget::APSTA;

  uint32_t now = 0;
  uint32_t bootTs = 0;          // when WiFi started at boot
  uint32_t lastActivityTs = 0;  // last HTTP / WS activity
  uint32_t apOpenTs = 0;        // last time the AP was (re)opened
  uint32_t staUpSinceTs = 0;    // STA got IP at, 0 if STA has no IP
  uint32_t staLostSinceTs = 0;  // STA lost at, 0 if not lost

  uint32_t bootWindowMs = 60000;
  uint32_t idleMs = 120000;
  uint32_t staLostMs = 15000;
};

inline bool wifiPolicyWithin(uint32_t now, uint32_t ts, uint32_t windowMs) {
  return ts != 0 && (uint32_t)(now - ts) < windowMs;
}

// True while something needs the AP: boot window, recent page activity,
// recent AP (re)open (button / fallback), or an open config page.
inline bool wifiPolicyApNeeded(const WifiPolicyIn& in) {
  return wifiPolicyWithin(in.now, in.bootTs, in.bootWindowMs) ||
         wifiPolicyWithin(in.now, in.lastActivityTs, in.idleMs) ||
         wifiPolicyWithin(in.now, in.apOpenTs, in.idleMs) || in.wsClient;
}

inline WifiTarget decideWifiTarget(const WifiPolicyIn& in) {
  if (!in.enabled) {
    return WifiTarget::APSTA;
  }

  bool apNeeded = wifiPolicyApNeeded(in);
  // In OSC mode an associated device may be receiving OSC over the AP
  if (in.oscEnabled && in.apStation) {
    apNeeded = true;
  }

  if (in.staUpSinceTs != 0) {
    // STA has an IP
    if (apNeeded) {
      return WifiTarget::APSTA;
    }
    if ((uint32_t)(in.now - in.staUpSinceTs) < WIFI_STA_STABLE_MS) {
      return in.current == WifiTarget::OFF ? WifiTarget::APSTA : in.current;
    }
    return WifiTarget::STA_ONLY;
  }

  // No STA. In STA-only mode give auto-reconnect a grace period before
  // re-opening the AP so the device never becomes unreachable.
  if (in.current == WifiTarget::STA_ONLY) {
    if (wifiPolicyWithin(in.now, in.staLostSinceTs, in.staLostMs)) {
      return WifiTarget::STA_ONLY;
    }
    return WifiTarget::APSTA;
  }

  // OSC needs the AP as its data path when there is no STA
  if (in.oscEnabled || apNeeded) {
    return WifiTarget::APSTA;
  }

  // MIDI only, no STA, idle: radio off (BLE stays up if enabled)
  return WifiTarget::OFF;
}

inline const char* wifiTargetName(WifiTarget t) {
  switch (t) {
    case WifiTarget::APSTA:
      return "APSTA";
    case WifiTarget::STA_ONLY:
      return "STA_ONLY";
    case WifiTarget::OFF:
      return "OFF";
  }
  return "?";
}
