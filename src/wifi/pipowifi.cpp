#include <wifi/pipowifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  pwm.setup();
  WiFi.setAutoReconnect(true);
  // allow to connect to (WHY SO WEAK?) wep networks
  WiFi.setMinSecurity(WIFI_AUTH_WEP);
  // prevent from the Wifi to sleep: avoid latency in websockets
  WiFi.setSleep(false);
  scan();
  APSTAMode();
};
void PipoWifi::scan() {
  int num = WiFi.scanNetworks(false, false, false, 500U);
  saveScanResult();
};
bool PipoWifi::startScan() {
  if (scanning)
    return false;
  scanning = true;
  return WiFi.scanNetworks(true, false, true, 300U) == WIFI_SCAN_RUNNING;
};
void PipoWifi::saveScanResult() {
  signals.clear();
  Serial.print("Saving scan results: ");
  Serial.println(WiFi.scanComplete());
  Serial.println(floor(rand() * 10000.));
  for (int i = 0; i < WiFi.scanComplete(); i++) {
    signals[WiFi.SSID(i)] = WiFi.RSSI(i);
  }
  WiFi.scanDelete();
  lastScan = millis();
};
bool PipoWifi::connect(bool disconnect) {
  status = CONNECTING;
  for (auto const& ssid : signals) {
    try {
      if (connect(ssid.first, disconnect)) {
        return true;
      }
    } catch (const std::exception& e) {
      continue;
    }
  }
  // if no success, switch to AP mode
  Serial.println("No one to connect to, switching to AP mode");
  APMode();
  return false;
};

bool PipoWifi::connect(String ssid, bool disconnect) {
  if (!pwm.hasSSID(ssid)) {
    return false;
  }
  return connect(ssid, pwm.getPassword(ssid), disconnect);
}

bool PipoWifi::connect(String ssid, String password, bool disconnect) {
  Serial.print("Connecting to " + ssid);
  Serial.println(" with password " + String(password));
  status = CONNECTING;
  if (disconnect) {
    WiFi.disconnect(true, true);
    vTaskDelay(pdMS_TO_TICKS(WIFI_DELAY));
  }
  if (WiFi.getMode() != WIFI_MODE_STA && WiFi.getMode() != WIFI_MODE_APSTA) {
    WiFi.mode(WIFI_MODE_STA);
  }
  int result = WiFi.begin(ssid, password);
  uint8_t timeoutClick = CONNECT_TIMEOUT / CHECK_TIMEOUT;
  while ((WiFi.status() != WL_CONNECTED) and --timeoutClick > 0) {
    vTaskDelay(pdMS_TO_TICKS(CHECK_TIMEOUT));
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to " + String(ssid.c_str()));
    Serial.println("IP " + WiFi.localIP().toString());
    pwm.add(ssid, password);
    pwm.promote(ssid);
    pwm.save();
    rssi = signals[ssid];
    status = CONNECTED;
    return true;
  }
  Serial.println("Failed to connect to " + String(ssid.c_str()));
  Serial.println("Error: " + String(result));
  return false;
};

bool PipoWifi::APMode() {
  Serial.println("Start AP mode");
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_AP);
  status = DISCONNECTED;
  vTaskDelay(pdMS_TO_TICKS(WIFI_DELAY));
  if (!configureAP()) {
    Serial.println("Failed to start AP mode");
    return false;
  }
  Serial.println("AP mode started");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  return true;
};

bool PipoWifi::APSTAMode() {
  if (WiFi.getMode() == WIFI_MODE_APSTA) {
    return true;
  }
  bool success = true;
  bool successConnect = true;
  bool wasConnected = status == CONNECTED;
  String previousSsid = WiFi.SSID();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_MODE_APSTA);
  vTaskDelay(pdMS_TO_TICKS(WIFI_DELAY));
  if (wasConnected) {
    successConnect =
        connect(previousSsid, pwm.getPassword(previousSsid), false);
  } else {
    successConnect = connect(false);
  }
  if (!successConnect) {
    Serial.println("APSTA: Failed to connect to network");
  }
  vTaskDelay(pdMS_TO_TICKS(500));
  success = configureAP();
  if (success) {
    Serial.println("APSTA mode started");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start APSTA mode");
  }
  return success & successConnect;
};

bool PipoWifi::configureAP() {
  getFreeSubNet();
  apIP = IPAddress(192, 168, subnetBase, 1);
  WiFi.softAPConfig(apIP, apIP, apMask);
  return WiFi.softAP("Pipo", "pipo1234", 6, false, 6);
}

bool PipoWifi::STAMode() {
  if (WiFi.getMode() == WIFI_MODE_STA && status == CONNECTED) {
    return true;
  }
  bool wasConnected = status == CONNECTED;
  String previousSsid = WiFi.SSID();
  WiFi.disconnect(true, true);
  vTaskDelay(pdMS_TO_TICKS(WIFI_DELAY));
  WiFi.mode(WIFI_MODE_STA);
  if (wasConnected) {
    return connect(previousSsid, pwm.getPassword(previousSsid), false);
  } else {
    return connect();
  }
};

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
  if (scanning) {
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
      return "Scanning";
    scanning = false;
    if (WiFi.scanComplete() == WIFI_SCAN_FAILED) {
      return "Scan failed";
    }
    saveScanResult();
  }
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

void PipoWifi::triggerRefreshRSSI() {
  shouldRefreshRSSI = true;
}
void PipoWifi::refreshRSSI() {
  // if (!shouldRefreshRSSI)
  //   return;
  rssi = WiFi.RSSI();
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

PipoWifi wifi;