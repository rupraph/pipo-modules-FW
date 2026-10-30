#include <wifi/pipowifi.h>
#include <osc/osc_handler.h>
#include "server/server.h"
#include <esp_wifi.h>
#include <esp_netif.h>

//TODO: Should move content from callback (only put flags)

void wifiTask(void* pvParameters) {
  // WiFi task is non-critical - don't subscribe to watchdog
  // unsigned long lastStackCheck = 0;k
  unsigned long serverPausedTime = 0;
  const unsigned long SERVER_PAUSE_TIMEOUT = 10000;  // 10 seconds max pause

  for (;;) {
    wifi.refresh();
    vTaskDelay(pdMS_TO_TICKS(500));

    // // Monitor stack usage every 30 seconds
    // if (millis() - lastStackCheck > 30000) {
    //   UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    //   log_i("wifiTask stack high water mark: %d bytes free",
    //         stackHighWaterMark);
    //   lastStackCheck = millis();
    // }

    // Track when server was paused
    if (!server.isRunning() && serverPausedTime == 0) {
      serverPausedTime = millis();
    }

    // Resume server if:
    // 1. AP is started (even if not fully configured yet), OR
    // 2. STA is connected (AP may be off in power-save mode), OR
    // 3. Server has been paused for more than timeout (failsafe)
    // Never while the radio is off in power-save mode.
    bool shouldResume =
        !wifiRadioOff &&
        (apStarted || staConnected ||
         (serverPausedTime > 0 &&
          (millis() - serverPausedTime) > SERVER_PAUSE_TIMEOUT));

    if (shouldResume && !server.isRunning()) {
      log_i("Resuming server (apStarted=%d, timeout=%d)", apStarted,
            serverPausedTime > 0 &&
                (millis() - serverPausedTime) > SERVER_PAUSE_TIMEOUT);
      server.resume();
      serverPausedTime = 0;
    }

    if (server.isRunning()) {
      serverPausedTime = 0;  // Reset if running
    }

    // RSSI is now read directly when needed via getRSSI()
  }
}

PipoWifi::PipoWifi() {};
void PipoWifi::setup() {
  log_i("Pipo WiFi setup");
  pwm.setup();

  // Register event handlers directly - standard ESP32 approach
  WiFi.onEvent(onWiFiReadyHandler, ARDUINO_EVENT_WIFI_READY);
  WiFi.onEvent(onScanDoneHandler, ARDUINO_EVENT_WIFI_SCAN_DONE);
  WiFi.onEvent(onSTAStartHandler, ARDUINO_EVENT_WIFI_STA_START);
  WiFi.onEvent(onSTAStopHandler, ARDUINO_EVENT_WIFI_STA_STOP);
  WiFi.onEvent(onSTAConnectedHandler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(onSTADisconnectedHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onSTAAuthModeChangeHandler,
               ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE);
  WiFi.onEvent(onSTAGotIPHandler, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(onSTAGotIP6Handler, ARDUINO_EVENT_WIFI_STA_GOT_IP6);
  WiFi.onEvent(onSTALostIPHandler, ARDUINO_EVENT_WIFI_STA_LOST_IP);
  WiFi.onEvent(onAPStartHandler, ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(onAPStopHandler, ARDUINO_EVENT_WIFI_AP_STOP);
  WiFi.onEvent(onAPStationConnectedHandler, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(onAPStationDisconnectedHandler,
               ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  WiFi.onEvent(onAPStationIPAssignedHandler,
               ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
  WiFi.onEvent(onAPProbeReqReceivedHandler,
               ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED);
  WiFi.onEvent(onAPGotIP6Handler, ARDUINO_EVENT_WIFI_AP_GOT_IP6);

  intentionalDisconnect = true;
  WiFi.disconnect(true, true);  // Ensure no previous connection persists
  WiFi.mode(
      WIFI_MODE_NULL);  // Reset Wi-Fi stack to prevent auto-starting in STA
  vTaskDelay(pdMS_TO_TICKS(100));  //
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.setAutoReconnect(true);
  // allow to connect to (WHY SO WEAK?) wep networks
  WiFi.setMinSecurity(WIFI_AUTH_WEP);
  // prevent from the Wifi to sleep: avoid latency in websockets. But removed because incompatible with BLE.
  scanning = true;
  log_i("WiFi scan network initiated");
  int num = WiFi.scanNetworks(true, false, false, 300U);
  bootTs = millis();
  lastPowerSwitchTs = bootTs;
  log_i("WiFi scan complete, setup done");
  if (DEBUG_HEAP)
    pipoDebugHeap("End setup pipowifi");
};
void PipoWifi::saveScanResult() {
  signals.clear();
  log_d("Saving scan results: %d networks found", WiFi.scanComplete());
  for (int i = 0; i < WiFi.scanComplete(); i++) {
    signals[WiFi.SSID(i)] = WiFi.RSSI(i);
  }
  WiFi.scanDelete();
  lastScan = millis();
};

// WiFi Event Handlers - Standard ESP32 approach
void onWiFiReadyHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] WIFI_READY");
}

void onScanDoneHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] SCAN_DONE");
  wifi.scanning = false;
  wifi.saveScanResult();
  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTAStartHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] STA_START");
  staStarted = true;
  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTAStopHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] STA_STOP");
  staStarted = false;
  wifi.next.ssid = "";
  wifi.next.password = "";
  osc.stop();  // STA interface stopped, stop UDP

  // If in APSTA mode and AP is configured, restart UDP for AP interface
  if (WiFi.getMode() == WIFI_MODE_APSTA && apConfigured) {
    Serial.println("  STA stopped but AP configured, restarting UDP for AP");
    vTaskDelay(pdMS_TO_TICKS(50));  // Brief delay
    osc.start();                    // Restart for AP interface
  }

  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTAConnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_i("[Event] STA_CONNECTED");
  wifi.status = PipoWifi::CONNECTED;
  staConnected = true;
  wifi.pwm.add(wifi.next.ssid, wifi.next.password);
  wifi.pwm.markAsConnected(wifi.next.ssid);
  wifi.needsSave = true;  // Mark that we need to save from task context
  wifi.rssi = wifi.signals[wifi.next.ssid];
  wifi.reconnectAttempts = 0;       // Reset counter on successful connection
  wifi.currentConnectingSSID = "";  // Clear tracking SSID
  wifi.next.ssid = "";
  wifi.next.password = "";
  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTADisconnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t reason = info.wifi_sta_disconnected.reason;
  log_w("[Event] STA_DISCONNECTED - Reason: %d", reason);

  // Check if this was an intentional disconnect
  if (wifi.intentionalDisconnect) {
    log_d("  Intentional disconnect, resetting counter");
    wifi.reconnectAttempts = 0;
    wifi.intentionalDisconnect = false;

    // If we have a new AP to connect to (switching networks), keep the credentials
    // Otherwise clear them (user-initiated disconnect)
    if (wifi.next.ssid.length() > 0) {
      log_i("  Have new AP queued (%s), will attempt connection",
            wifi.next.ssid.c_str());
    } else {
      log_d("  No new AP queued, staying disconnected");
    }
  } else {
    // Unintentional disconnect - connection failed or dropped
    // Check if this was the AP we were trying to connect to
    if (strcmp((char*)info.wifi_sta_disconnected.ssid,
               wifi.currentConnectingSSID.c_str()) == 0) {
      wifi.reconnectAttempts++;
      log_w("  Reconnect attempt: %d/%d", wifi.reconnectAttempts,
            wifi.MAX_RECONNECT_ATTEMPTS);

      if (wifi.reconnectAttempts >= wifi.MAX_RECONNECT_ATTEMPTS) {
        log_w("  Max reconnect attempts reached for %s, clearing credentials",
              wifi.currentConnectingSSID.c_str());
        // Don't save these credentials - connection failed
        wifi.next.ssid = "";
        wifi.next.password = "";
        wifi.currentConnectingSSID = "";
        wifi.reconnectAttempts = 0;
        WiFi.setAutoReconnect(false);
      } else {
        wifi.lastReconnectAttempt = millis();
      }
    }
  }

  wifi.status = PipoWifi::DISCONNECTED;
  staConnected = false;
  wifi.markStaDown();
  osc.stop();  // STA disconnected, stop UDP

  // If in APSTA mode and AP is configured, restart UDP for AP interface
  if (WiFi.getMode() == WIFI_MODE_APSTA && apConfigured) {
    Serial.println("  STA lost but AP configured, restarting UDP for AP");
    vTaskDelay(pdMS_TO_TICKS(50));  // Brief delay
    osc.start();                    // Restart for AP interface
  }

  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTAAuthModeChangeHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] STA_AUTHMODE_CHANGE");
}

void onSTAGotIPHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_i("[Event] STA_GOT_IP - IP: %s", WiFi.localIP().toString().c_str());
  wifi.status = PipoWifi::CONNECTED;
  wifi.markStaUp();
  osc.start();               // Network ready, start UDP
  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTAGotIP6Handler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_i("[Event] STA_GOT_IP6 - IP: %s", WiFi.localIP().toString().c_str());
  wifi.status = PipoWifi::CONNECTED;
  osc.start();               // Network ready with IPv6, start UDP
  wifi.stateChanged = true;  // Signal state machine to run
}

void onSTALostIPHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_w("[Event] STA_LOST_IP");
  wifi.status = PipoWifi::DISCONNECTED;
  wifi.markStaDown();
  osc.stop();  // Network lost, stop UDP

  // If in APSTA mode and AP is configured, restart UDP for AP interface
  if (WiFi.getMode() == WIFI_MODE_APSTA && apConfigured) {
    Serial.println("  STA lost IP but AP configured, restarting UDP for AP");
    vTaskDelay(pdMS_TO_TICKS(50));  // Brief delay
    osc.start();                    // Restart for AP interface
  }

  wifi.stateChanged = true;  // Signal state machine to run
}

void onAPStartHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] AP_START - apStarted flag: %d", true);
  apStarted = true;
  // Note: apStarted means the AP has started, but not necessarily configured yet
  // Wait for configureAP() to set apConfigured, then start UDP
  wifi.stateChanged = true;  // Signal state machine to run
}

void onAPStopHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] AP_STOP");
  apStarted = false;
  apConfigured = false;
  apConnected = false;
  // Keep UDP running when the AP is dropped but STA still carries OSC
  if (!staConnected) {
    osc.stop();  // AP stopped, stop UDP
  }
  wifi.stateChanged = true;  // Signal state machine to run
}

void onAPStationConnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_i("[Event] AP client connected (%d connected)",
        WiFi.softAPgetStationNum());
  apConnected = true;
}

void onAPStationDisconnectedHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  int remaining = WiFi.softAPgetStationNum();
  log_i("[Event] AP client disconnected (%d connected)", remaining);
  apConnected = remaining > 0;
  if (remaining == 0) {
    wifi.markApClientLeft();  // give the user time to reconnect
  }
}

void onAPStationIPAssignedHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] AP_STAIPASSIGNED - apConnected: -> true");
  apConnected = true;
  osc.start();  // AP client ready, start UDP
}

void onAPProbeReqReceivedHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_v("[Event] AP_PROBEREQRECVED");
}

void onAPGotIP6Handler(WiFiEvent_t event, WiFiEventInfo_t info) {
  log_d("[Event] AP_GOT_IP6");
  osc.start();  // AP ready with IPv6, start UDP
}

bool PipoWifi::connect() {
  // Only try to connect to the last connected network if it's available
  String lastSSID = pwm.getLastConnectedSSID();

  if (lastSSID.length() == 0) {
    log_i("No last connected network stored, not attempting auto-connect");
    status = DISCONNECTED;
    return false;
  }

  // Check if last connected network is available in current scan
  if (signals.find(lastSSID) == signals.end()) {
    log_i("Last connected network '%s' not in range", lastSSID.c_str());
    status = DISCONNECTED;
    return false;
  }

  // Try to connect to the last connected network
  log_i("Attempting to reconnect to last used network: %s", lastSSID.c_str());
  return connect(lastSSID);
};

bool PipoWifi::connect(String ssid) {
  if (!pwm.hasSSID(ssid)) {
    return false;
  }
  return connect(ssid, pwm.getPassword(ssid));
}

bool PipoWifi::connect(String ssid, String password) {
  status = CONNECTING;
  next.ssid = ssid;
  next.password = password;

  // Only reset counter if this is a new SSID, not a retry of the same one
  if (currentConnectingSSID != ssid) {
    currentConnectingSSID = ssid;  // Track SSID for reconnect attempts
    reconnectAttempts = 0;         // Reset counter for new connection attempt
  }
  // If it's the same SSID, keep the existing reconnectAttempts count

  lastReconnectAttempt = millis();
  WiFi.setAutoReconnect(true);  // Re-enable auto-reconnect for new connection
  log_i("Connecting to WiFi: %s (attempt %d)", ssid.c_str(),
        reconnectAttempts + 1);
  WiFi.begin(ssid.c_str(), password.c_str());
  return true;
};

//Todo: context issue. leds cannot be started from there
bool PipoWifi::configureAP() {
  getFreeSubNet();
  apIP = IPAddress(192, 168, subnetBase, 1);
  WiFi.softAPConfig(apIP, apIP, apMask);
  string apName = "Pipo-" + config.general_config["PipoName"].as<string>();
  log_i("Starting AP: %s", apName.c_str());
  apStarted = WiFi.softAP(apName.c_str(), "pipo1234", 6, false, 6);
  if (apStarted) {
    log_i("AP started successfully");
    apConfigured = true;
    // AP has IP address now, start UDP for communication
    // osc.start();
    //hwui.start_blink(WIFI_LED, WIFI_AP_PULSE_TIME, 0.2);
  } else {
    log_e("Failed to start AP");
    apConfigured = false;
  }
  return apStarted;
}

PipoWifi::PipoWifiStatus PipoWifi::getStatus() {
  return status;
};

String PipoWifi::state() {
  String res = "";
  wifi_mode_t mode = WiFi.getMode();
  switch (mode) {
    case WIFI_MODE_APSTA:
      res = "APSTA";
      break;
    case WIFI_MODE_STA:
      res = "STA";
      break;
    case WIFI_MODE_AP:
      res = "AP";
      break;
    case WIFI_MODE_NULL:
      res = "NULL";
      break;
    case WIFI_MODE_MAX:
      res = "MAX";
      break;
  }
  res += "\n";
  switch (status) {
    case DISCONNECTED:
      res += "DISCONNECTED";
      break;
    case CONNECTING:
      res += "CONNECTING";
      break;
    case CONNECTED:
      res += "CONNECTED";
      res += " ";
      res += WiFi.localIP().toString().c_str();
      res += " ";
      res += WiFi.SSID().c_str();
      break;
    default:
      res += "UNKNWON";
  }
  if ((mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) && apStarted) {
    res += "\nAP ";
    res += apIP.toString();
  }
  return res;
}
String PipoWifi::availableNetworks() {
  String res = "lastScan:";
  res += String(lastScan);
  res += "\n";
  for (auto const& ssid : signals) {
    res += "\"";
    res += ssid.first;
    res += "\"";
    res += " ";
    res += String(ssid.second);
    res += WiFi.SSID() == ssid.first ? " 1" : " 0";
    res += pwm.hasSSID(ssid.first) ? " 1" : " 0";
    res += "\n";
  }
  return res;
}

String PipoWifi::ssid() {
  return WiFi.SSID();
}

bool PipoWifi::isScanning() {
  return scanning;
}
bool PipoWifi::ready() {
  wifi_mode_t mode = WiFi.getMode();
  bool notScanningOrConnecting = !scanning && status != CONNECTING;
  bool apStaReady = mode == WIFI_MODE_APSTA && staStarted && apStarted;
  bool staReady = mode == WIFI_MODE_STA && staStarted;
  bool apReady = (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) && apStarted;
  return notScanningOrConnecting && (apStaReady || staReady || apReady);
}

int8_t PipoWifi::getRSSI() {
  // Return live RSSI value if connected, otherwise return cached value
  if (WiFi.getMode() != WIFI_MODE_NULL && WiFi.status() == WL_CONNECTED) {
    return WiFi.RSSI();
  }
  return rssi;  // Fallback to cached scan result
}

void PipoWifi::getFreeSubNet() {
  subnetBase = MIN_SUBNET;
  for (auto const& ssid : signals) {
    if (!ssid.first.startsWith("Pipo"))
      continue;
    subnetBase++;
  }
}
void PipoWifi::step() {
  if (scanning || status == CONNECTING) {
    log_d("Step: scanning or connecting, skipping");
    return;
  }

  log_d("Step: mode=%d next.mode=%d apStarted=%d staStarted=%d status=%d",
        WiFi.getMode(), next.mode, apStarted, staStarted, status);
  wifi_mode_t mode = WiFi.getMode();

  if (next.mode != WiFi.getMode()) {
    osc.stop();  // Stop OSC before changing WiFi mode
    intentionalDisconnect = true;
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_NULL);
    // vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.mode(next.mode);
    wifiStaOnly = next.mode == WIFI_MODE_STA;
    apStarted = false;
    apConfigured = false;
    staStarted = false;
  }
  if (staStarted && status != CONNECTING && (status != CONNECTED)) {
    // STA is ready, and we need to connect
    // User manually requested connection (next.ssid is set)
    if (next.ssid.length() && next.password.length()) {
      if (connect(next.ssid, next.password)) {
        return;
      }
    }
    if (next.ssid.length()) {
      if (connect(next.ssid)) {
        return;
      }
    }
    // Only auto-connect if user hasn't requested disconnect
    if (pwm.shouldAutoConnect()) {
      if (connect()) {
        return;
      }
    } else {
      log_i("User requested disconnect - skipping auto-connect");
    }
  }

  log_d("Step: Checking AP - mode=%d apStarted=%d apConfigured=%d", mode,
        apStarted, apConfigured);

  if (!apConfigured && (mode == WIFI_MODE_AP ||
                        mode == WIFI_MODE_APSTA && status != CONNECTING)) {
    // we are in AP mode, need to configure it
    log_d("Configuring AP");
    configureAP();
  }
}
void PipoWifi::refresh() {
  if (scanning || status == CONNECTING)
    return;

  // Save password manager changes from task context (safe to write to flash here)
  if (needsSave) {
    pwm.save();
    needsSave = false;
  }

  // Run state machine if events triggered a state change
  // This ensures step() only runs in task context, never from event handlers
  if (stateChanged) {
    stateChanged = false;
    step();
  }

  powerTick();

  wifi_mode_t prevMode = WiFi.getMode();

  if (next.shouldScan) {
    log_d("WiFi: Starting network scan");
    scanning = WiFi.scanNetworks(true, false, true, 300U) == WIFI_SCAN_RUNNING;
    next.shouldScan = false;
  }
}
void PipoWifi::setMode(wifi_mode_t mode) {
  wifi_mode_t prevMode = WiFi.getMode();
  if (prevMode == mode)
    return;
  // next.transition = computeWiFiTransition(prevMode, mode);
  next.mode = mode;
}
void PipoWifi::setSSID(String ssid) {
  if (next.ssid == ssid)
    return;
  next.ssid = ssid;
}
void PipoWifi::setPassword(String password) {
  if (next.password == password)
    return;
  next.password = password;
}
void PipoWifi::requestScan() {
  next.shouldScan = true;
}
void PipoWifi::forgetNetwork(String ssid) {
  pwm.remove(ssid);
  pwm.save();
}

void PipoWifi::disconnect() {
  log_i("User-initiated disconnect");
  pwm.setDisconnectRequest();  // Set flag (no save in method)
  pwm.save();  // Safe to save here - called from HTTP handler task context

  // Clear any pending connection attempts to avoid connecting after disconnect
  next.ssid = "";
  next.password = "";
  currentConnectingSSID = "";

  intentionalDisconnect = true;
  WiFi.disconnect();
}

// ---------------------------------------------------------------------------
// Battery-saving mode
// decideWifiTarget() (wifi_policy.h) picks APSTA / STA_ONLY / OFF; the methods
// below apply it. Mode changes only happen here, from refresh() in wifiTask.
// ---------------------------------------------------------------------------

void PipoWifi::markStaUp() {
  staUpSinceTs = millis();
  staLostSinceTs = 0;
}

void PipoWifi::markStaDown() {
  if (staUpSinceTs != 0 || staLostSinceTs == 0) {
    staLostSinceTs = millis();
  }
  staUpSinceTs = 0;
}

// True if the OSC target is a device currently connected to the AP (its
// DHCP lease matches OSC_IP). Being on the AP subnet is not enough: the
// default OSC_IP is on it, which would otherwise hold the AP forever.
bool PipoWifi::oscTargetOnAp() {
  IPAddress target;
  if (!apStarted ||
      !target.fromString(config.general_config["OSC_IP"] | "")) {
    return false;
  }
  wifi_sta_list_t stations;
  if (esp_wifi_ap_get_sta_list(&stations) != ESP_OK || stations.num == 0) {
    return false;
  }
  esp_netif_pair_mac_ip_t pairs[ESP_WIFI_MAX_CONN_NUM];
  for (int i = 0; i < stations.num; i++) {
    memcpy(pairs[i].mac, stations.sta[i].mac, 6);
    pairs[i].ip.addr = 0;
  }
  esp_netif_t* apNetif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
  if (apNetif == nullptr ||
      esp_netif_dhcps_get_clients_by_mac(apNetif, stations.num, pairs) !=
          ESP_OK) {
    return false;
  }
  for (int i = 0; i < stations.num; i++) {
    if (pairs[i].ip.addr == (uint32_t)target) {
      return true;
    }
  }
  return false;
}

void PipoWifi::markApClientLeft() {
  // Ignore clients kicked by our own AP shutdown (disableAP / radioOff set
  // the new state before stopping the AP), or the AP would reopen at once
  if (powerState() == WifiTarget::APSTA) {
    apClientLeftTs = millis();
  }
}

WifiTarget PipoWifi::powerState() {
  if (wifiRadioOff) {
    return WifiTarget::OFF;
  }
  return next.mode == WIFI_MODE_STA ? WifiTarget::STA_ONLY : WifiTarget::APSTA;
}

void PipoWifi::powerTick() {
  uint32_t now = millis();
  JsonDocument& g = config.general_config;
  WifiTarget current = powerState();

  if (apRequested) {
    apRequested = false;
    log_i("WiFi power: AP requested by button");
    apOpenTs = now;
    lastPowerSwitchTs = 0;  // user action: skip dwell
  }

  // Keys may be missing from older configs / presets: fall back to defaults
  if (!(g["Wifi_ps_ena"] | false)) {
    // Turning the feature off restores APSTA once, then leaves the mode alone
    if (powerSaveWasEnabled) {
      log_i("WiFi power: battery saving disabled");
      if (current != WifiTarget::APSTA) {
        setPowerTarget(current, WifiTarget::APSTA);
      }
    }
    powerSaveWasEnabled = false;
    return;
  }
  if (!powerSaveWasEnabled) {
    log_i("WiFi power: battery saving enabled (boot %ds, idle %ds)",
          (int)(g["Wifi_ap_boot_s"] | 60), (int)(g["Wifi_ap_idle_s"] | 120));
  }
  powerSaveWasEnabled = true;

  WifiPolicyIn in;
  in.enabled = true;
  in.oscEnabled = g["OSC_ENA"] | false;
  in.bootWindowMs = (uint32_t)(g["Wifi_ap_boot_s"] | 60) * 1000;
  in.idleMs = (uint32_t)(g["Wifi_ap_idle_s"] | 120) * 1000;
  in.staLostMs = (uint32_t)(g["Wifi_sta_lost_s"] | 15) * 1000;
  in.oscTargetOnAp = oscTargetOnAp();
  in.current = current;
  in.now = now;
  in.bootTs = bootTs;
  in.lastActivityTs = lastActivityTs;
  in.apOpenTs = apOpenTs;
  in.apClientLeftTs = apClientLeftTs;
  in.staUpSinceTs = staUpSinceTs;
  in.staLostSinceTs = staLostSinceTs;

  logPowerHold(in);

  WifiTarget target = decideWifiTarget(in);
  if (target == current ||
      (uint32_t)(now - lastPowerSwitchTs) < POWER_MIN_DWELL_MS) {
    return;
  }
  // Don't switch while a config upload / user mode change paused the server
  if (current != WifiTarget::OFF && !server.isRunning()) {
    return;
  }
  setPowerTarget(current, target);
}

// Logs what keeps the AP on whenever it changes, so the countdown to the AP
// turning off can be followed on the serial monitor
void PipoWifi::logPowerHold(const WifiPolicyIn& in) {
  // Page counts as in use while heartbeats keep arriving (sent every 10 s)
  bool pageActive = wifiPolicyWithin(in.now, in.lastActivityTs, 25000);
  if (pageActive != pageWasActive) {
    if (pageActive) {
      log_i("WiFi power: config page in use");
    } else {
      log_i("WiFi power: config page idle");
    }
    pageWasActive = pageActive;
  }

  uint32_t msLeft = 0;
  WifiApHold hold = wifiPolicyApHold(in, &msLeft);
  const char* why = wifiApHoldName(hold);
  if (hold == WifiApHold::NONE && in.oscEnabled && in.staUpSinceTs == 0) {
    why = "OSC with no known network";
  }
  if (why != lastHoldReason) {
    if (msLeft > 0) {
      log_i("WiFi power: AP held by %s (%us left)", why,
            (unsigned)(msLeft / 1000));
    } else {
      log_i("WiFi power: AP held by %s", why);
    }
    lastHoldReason = why;
  }
}

void PipoWifi::setPowerTarget(WifiTarget from, WifiTarget to) {
  log_i("WiFi power: %s -> %s", wifiTargetName(from), wifiTargetName(to));
  uint32_t now = millis();

  if (from == WifiTarget::OFF) {
    radioOn();
  }
  switch (to) {
    case WifiTarget::APSTA:
      if (from == WifiTarget::STA_ONLY) {
        enableAP();
      }
      apOpenTs = now;  // a re-opened AP gets a full idle window
      break;
    case WifiTarget::STA_ONLY:
      disableAP();
      break;
    case WifiTarget::OFF:
      radioOff();
      break;
  }
  lastPowerSwitchTs = now;
}

void PipoWifi::enableAP() {
  log_i("Enabling AP (APSTA)");
  next.mode = WIFI_MODE_APSTA;  // keep step() from doing a full mode reset
  wifiStaOnly = false;
  apConfigured = false;
  WiFi.enableAP(true);
  configureAP();
}

void PipoWifi::disableAP() {
  log_i("Disabling AP (STA only, modem sleep)");
  next.mode = WIFI_MODE_STA;  // keep step() from doing a full mode reset
  wifiStaOnly = true;
  WiFi.softAPdisconnect(true);
  // Modem sleep is only effective without the AP
  WiFi.setSleep(WIFI_PS_MIN_MODEM);
}

void PipoWifi::radioOff() {
  log_i("Turning WiFi radio off");
  wifiRadioOff = true;
  wifiStaOnly = false;
  osc.stop();
  server.pause();
  next.mode = WIFI_MODE_NULL;
  if (staConnected) {
    intentionalDisconnect = true;
  }
  status = DISCONNECTED;
  WiFi.mode(WIFI_OFF);
  staStarted = false;
  staConnected = false;
  apStarted = false;
  apConfigured = false;
  apConnected = false;
  staUpSinceTs = 0;
  staLostSinceTs = 0;
}

void PipoWifi::radioOn() {
  log_i("Turning WiFi radio on (APSTA)");
  next.mode = WIFI_MODE_APSTA;
  WiFi.mode(WIFI_MODE_APSTA);
  wifiRadioOff = false;  // lets wifiTask resume the server once AP starts
  // Fresh scan so step() can auto-connect to a saved STA
  requestScan();
}

PipoWifi wifi;