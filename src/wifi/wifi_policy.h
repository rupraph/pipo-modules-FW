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
  bool oscTargetOnAp = false;  // OSC_IP is a device connected to the AP
  WifiTarget current = WifiTarget::APSTA;

  uint32_t now = 0;
  uint32_t bootTs = 0;          // when WiFi started at boot
  uint32_t lastActivityTs = 0;  // last page use (visible heartbeat, edits)
  uint32_t apOpenTs = 0;        // last time the AP was (re)opened
  uint32_t apClientLeftTs = 0;  // last time the AP's last client left
  uint32_t staUpSinceTs = 0;    // STA got IP at, 0 if STA has no IP
  uint32_t staLostSinceTs = 0;  // STA lost at, 0 if not lost

  uint32_t bootWindowMs = 60000;
  uint32_t idleMs = 120000;
  uint32_t staLostMs = 15000;
};

inline bool wifiPolicyWithin(uint32_t now, uint32_t ts, uint32_t windowMs) {
  return ts != 0 && (uint32_t)(now - ts) < windowMs;
}

// What currently needs the AP. A device merely associated to the AP does not
// count: a phone left on the AP after configuring gets disconnected once the
// idle time is over.
enum class WifiApHold : uint8_t {
  NONE,
  BOOT,        // boot window
  PAGE,        // recent page use (a visible page sends a heartbeat)
  REOPEN,      // AP recently (re)opened (button / fallback)
  CLIENT_LEFT,  // last AP client recently left: time to reconnect
  OSC_TARGET,  // the OSC target is a device on the AP: AP is the data path
};

// Returns the hold with the most time left (msLeft = 0 for untimed holds)
inline WifiApHold wifiPolicyApHold(const WifiPolicyIn& in,
                                   uint32_t* msLeft = nullptr) {
  WifiApHold hold = WifiApHold::NONE;
  uint32_t left = 0;
  if (in.oscEnabled && in.oscTargetOnAp) {
    hold = WifiApHold::OSC_TARGET;
  } else {
    auto consider = [&](uint32_t ts, uint32_t windowMs, WifiApHold h) {
      if (wifiPolicyWithin(in.now, ts, windowMs)) {
        uint32_t l = windowMs - (uint32_t)(in.now - ts);
        if (l > left) {
          left = l;
          hold = h;
        }
      }
    };
    consider(in.bootTs, in.bootWindowMs, WifiApHold::BOOT);
    consider(in.lastActivityTs, in.idleMs, WifiApHold::PAGE);
    consider(in.apOpenTs, in.idleMs, WifiApHold::REOPEN);
    consider(in.apClientLeftTs, in.idleMs, WifiApHold::CLIENT_LEFT);
  }
  if (msLeft) {
    *msLeft = left;
  }
  return hold;
}

inline const char* wifiApHoldName(WifiApHold h) {
  switch (h) {
    case WifiApHold::NONE:
      return "nothing";
    case WifiApHold::BOOT:
      return "boot window";
    case WifiApHold::PAGE:
      return "config page use";
    case WifiApHold::REOPEN:
      return "AP reopen";
    case WifiApHold::CLIENT_LEFT:
      return "AP client left";
    case WifiApHold::OSC_TARGET:
      return "OSC target connected to AP";
  }
  return "?";
}

inline WifiTarget decideWifiTarget(const WifiPolicyIn& in) {
  if (!in.enabled) {
    return WifiTarget::APSTA;
  }

  bool apNeeded = wifiPolicyApHold(in) != WifiApHold::NONE;

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
