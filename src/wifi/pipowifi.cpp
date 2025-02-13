#include <wifi/pipowifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  pwm.setup();
  WiFi.onEvent(std::bind(&PipoWifi::handleWiFiEvent, this,
                         std::placeholders::_1, std::placeholders::_2));
  WiFi.disconnect(true, true);  // Ensure no previous connection persists
  WiFi.mode(
      WIFI_MODE_NULL);  // Reset Wi-Fi stack to prevent auto-starting in STA
  vTaskDelay(pdMS_TO_TICKS(100));  //
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.setAutoReconnect(true);
  // allow to connect to (WHY SO WEAK?) wep networks
  WiFi.setMinSecurity(WIFI_AUTH_WEP);
  // prevent from the Wifi to sleep: avoid latency in websockets
  WiFi.setSleep(false);
  scanning = true;
  int num = WiFi.scanNetworks(false, false, false, 500U);
};
void PipoWifi::saveScanResult() {
  signals.clear();
  Serial.print("Saving scan results: ");
  Serial.println(WiFi.scanComplete());
  for (int i = 0; i < WiFi.scanComplete(); i++) {
    signals[WiFi.SSID(i)] = WiFi.RSSI(i);
  }
  WiFi.scanDelete();
  lastScan = millis();
};

void PipoWifi::handleWiFiEvent(arduino_event_id_t event,
                               arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
      Serial.println("WiFi Ready!");
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      Serial.println("WiFi Scan completed");
      scanning = false;
      saveScanResult();
      step();
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      staStarted = true;
      step();
      break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
      staStarted = false;
      next.ssid = "";
      next.password = "";
      step();
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("STA CONNECTED!");
      status = CONNECTED;
      pwm.add(next.ssid, next.password);
      pwm.promote(next.ssid);
      pwm.save();
      rssi = signals[next.ssid];
      next.ssid = "";
      next.password = "";
      isChangingAP = false;
      step();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("STA DISCONNECTED!");
      uint8_t reason = info.wifi_sta_disconnected.reason;
      // we disconnected from the asked AP: means wrong credentials,
      // erase the ssid and password to allow fallback to other APs
      if (strcmp((char*)info.wifi_sta_disconnected.ssid, next.ssid.c_str()) ==
          0) {
        next.ssid = "";
        next.password = "";
      }
      isChangingAP = false;
      status = DISCONNECTED;
      step();
      break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
      Serial.println("STA AUTHMODE CHANGE!");
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
      Serial.println("STA GOT IP!");
      status = CONNECTED;
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      step();
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("STA LOST IP!");
      Serial.println("WiFi disconnected!");
      status = DISCONNECTED;
      step();
      break;
    case ARDUINO_EVENT_WIFI_AP_START:
      apStarted = true;
      Serial.println("AP START!");
      step();
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      apStarted = false;
      Serial.println("AP STOP!");
      step();
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("APSTA CONNECTED!");
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("APSTA DISCONNECTED!");
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.println("APSTA IPASSIGNED!");
      break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
      Serial.println("STA PROB!");
      break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
      Serial.println("STA AP GOT IPV6!");
      break;
    case ARDUINO_EVENT_WIFI_FTM_REPORT:
      Serial.println("STA WIFI FTM REPORT!");
      break;
    default:
      Serial.print("Unknown event ");
      Serial.println(event);
      break;
  }
}

bool PipoWifi::connect() {
  status = CONNECTING;
  for (auto const& ssid : signals) {
    try {
      if (connect(ssid.first)) {
        return true;
      }
    } catch (const std::exception& e) {
      continue;
    }
  }
  status = DISCONNECTED;
  return false;
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
  WiFi.begin(ssid.c_str(), password.c_str());
  return true;
};

bool PipoWifi::configureAP() {
  getFreeSubNet();
  apIP = IPAddress(192, 168, subnetBase, 1);
  WiFi.softAPConfig(apIP, apIP, apMask);
  apStarted = WiFi.softAP("Pipo", "pipo1234", 6, false, 6);
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

  switch (status) {
    case DISCONNECTED:
      res += " DISCONNECTED";
      break;
    case CONNECTING:
      res += " CONNECTING";
      break;
    case CONNECTED:
      res += " CONNECTED";
      res += " ";
      res += WiFi.localIP().toString().c_str();
      res += " ";
      res += WiFi.SSID().c_str();
      break;
    default:
      res += " UNKNWON";
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
  // Serial.print("Ready? ");
  // Serial.print(mode);
  // Serial.print(" status ");
  // Serial.print(status);
  // Serial.print(" sta ");
  // Serial.print(staStarted);
  // Serial.print(" ap ");
  // Serial.println(apStarted);
  return notScanningOrConnecting && (apStaReady || staReady || apReady);
}

int8_t PipoWifi::getRSSI() {
  return rssi;
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
  if (scanning)
    return;

  Serial.println("Step");
  wifi_mode_t mode = WiFi.getMode();
  Serial.print("Change mode? ");
  Serial.print(next.mode);
  Serial.print(" mode ");
  Serial.print(mode);
  Serial.print(" apStarted ");
  Serial.print(apStarted);
  Serial.print(" staStarted ");
  Serial.print(staStarted);
  Serial.print(" status ");
  Serial.println(status);

  if (next.mode != WiFi.getMode()) {
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
    WiFi.mode(next.mode);
    apStarted = false;
    staStarted = false;
  }
  if (staStarted && status != CONNECTING && (status != CONNECTED)) {
    // STA is ready, and we need to connect
    if (next.ssid.length() && next.password.length() &&
        connect(next.ssid, next.password)) {
      return;
    }
    if (next.ssid.length() && connect(next.ssid)) {
      return;
    }
    if (connect()) {
      return;
    }
    // There might have been no one to connect to, we need to start AP anyway
    Serial.println("No one to connect to, starting AP");
  }
  Serial.print("Should AP? ");
  Serial.print(mode);
  Serial.print(" apStarted ");
  Serial.print(apStarted);
  Serial.print(" staStarted ");
  Serial.print(staStarted);
  Serial.print(" status ");
  Serial.println(status);

  if (!apStarted && (mode == WIFI_MODE_AP ||
                     mode == WIFI_MODE_APSTA && status != CONNECTING)) {
    // we are in AP mode, need to configure it
    Serial.println("Configure AP");
    configureAP();
  }
}
void PipoWifi::refresh() {
  if (scanning || status == CONNECTING)
    return;
  wifi_mode_t prevMode = WiFi.getMode();

  if (!scanning && next.shouldScan) {
    Serial.println("Scan");
    scanning = WiFi.scanNetworks(true, false, true, 300U) == WIFI_SCAN_RUNNING;
    next.shouldScan = false;
  } else if (next.shouldRSSI && !scanning) {
    Serial.println("RSSI");
    rssi = WiFi.RSSI();
    next.shouldRSSI = false;
  } else if (isChangingAP && status == CONNECTED) {
    Serial.println("Disconnect");
    WiFi.disconnect();
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
  isChangingAP = true;
}
void PipoWifi::setPassword(String password) {
  if (next.password == password)
    return;
  next.password = password;
  isChangingAP = true;
}
void PipoWifi::requestScan() {
  next.shouldScan = true;
}
void PipoWifi::requestRSSI() {
  next.shouldRSSI = true;
}

PipoWifi wifi;