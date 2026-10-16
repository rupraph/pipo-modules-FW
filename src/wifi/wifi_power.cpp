#include "wifi/wifi_power.h"
#include "wifi/pipowifi.h"
#include "server/server.h"
#include "server/pipo_socket.h"
#include "hw_ui.h"

PipoWifiPower wifiPower;

void PipoWifiPower::begin() {
  bootTs = millis();
  lastTransitionTs = bootTs;
  current = WifiTarget::APSTA;
}

void PipoWifiPower::onStaUp() {
  staUpSinceTs = millis();
  staLostSinceTs = 0;
}

void PipoWifiPower::onStaDown() {
  if (staUpSinceTs != 0 || staLostSinceTs == 0) {
    staLostSinceTs = millis();
  }
  staUpSinceTs = 0;
}

void PipoWifiPower::readConfig(WifiPolicyIn& in) {
  JsonDocument& g = config.general_config;
  // Keys may be missing from older configs / presets: fall back to defaults
  in.enabled = g["Wifi_ps_ena"] | false;
  in.oscEnabled = g["OSC_ENA"] | false;
  in.bootWindowMs = (uint32_t)(g["Wifi_ap_boot_s"] | 60) * 1000;
  in.idleMs = (uint32_t)(g["Wifi_ap_idle_s"] | 120) * 1000;
  in.staLostMs = (uint32_t)(g["Wifi_sta_lost_s"] | 15) * 1000;
  hwui.pause_sw.set_long_press_ms(g["Wifi_ap_btn_ms"] | 2000);
}

void PipoWifiPower::tick() {
  uint32_t now = millis();

  if (apRequested) {
    apRequested = false;
    log_i("WiFi power: AP requested by button");
    apOpenTs = now;
    lastTransitionTs = 0;  // user action: skip dwell
  }

  WifiPolicyIn in;
  readConfig(in);
  in.current = current;
  in.now = now;
  in.bootTs = bootTs;
  in.lastActivityTs = lastActivityTs;
  in.apOpenTs = apOpenTs;
  in.staUpSinceTs = staUpSinceTs;
  in.staLostSinceTs = staLostSinceTs;
  in.wsClient = pipoSocket.hasClient();
  in.apStation = apStarted && WiFi.softAPgetStationNum() > 0;

  WifiTarget target = decideWifiTarget(in);
  if (target == current) {
    return;
  }
  if ((uint32_t)(now - lastTransitionTs) < MIN_DWELL_MS) {
    return;
  }
  // Don't switch while a config upload / user mode change paused the server
  if (current != WifiTarget::OFF && !server.isRunning()) {
    return;
  }
  apply(target);
}

void PipoWifiPower::apply(WifiTarget target) {
  log_i("WiFi power: %s -> %s", wifiTargetName(current),
        wifiTargetName(target));
  uint32_t now = millis();

  if (current == WifiTarget::OFF) {
    wifi.radioOn();
  }
  switch (target) {
    case WifiTarget::APSTA:
      if (current == WifiTarget::STA_ONLY) {
        wifi.enableAP();
      }
      apOpenTs = now;  // a re-opened AP gets a full idle window
      break;
    case WifiTarget::STA_ONLY:
      wifi.disableAP();
      break;
    case WifiTarget::OFF:
      staUpSinceTs = 0;
      staLostSinceTs = 0;
      wifi.radioOff();
      break;
  }
  current = target;
  lastTransitionTs = now;
}
