#include <wifi/pipowifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  pwm.setup();
  WiFi.setAutoReconnect(true);
  STAMode();
  // APMode();
};
void PipoWifi::scan() {
  int num = WiFi.scanNetworks(false, false, false, 300U);
  Serial.println("scan done");
  for (int i = 0; i < num; i++) {
    int rssiperc = WiFi.RSSI(i);
    signals[WiFi.SSID(i)] = rssiperc;
  }
};
bool PipoWifi::connect() {
  Serial.println("Connect...");
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
  // if no success, switch to AP mode
  Serial.println("No one to connect to, switching to AP mode");
  APMode();
  return false;
};

bool PipoWifi::connect(String ssid) {
  if (!pwm.hasSSID(ssid)) {
    return false;
  }
  return connect(ssid, pwm.getPassword(ssid));
}

bool PipoWifi::connect(String ssid, String password, bool disconnect) {
  Serial.print("Connecting to " + ssid);
  Serial.println(" with password " + String(password));
  status = CONNECTING;
  if (disconnect) {
    WiFi.disconnect(true, true);
  }
  if (WiFi.getMode() != WIFI_MODE_STA && WiFi.getMode() != WIFI_MODE_APSTA) {
    WiFi.mode(WIFI_MODE_STA);
  }
  int result = WiFi.begin(ssid, password);
  uint8_t timeoutClick = CONNECT_TIMEOUT / CHECK_TIMEOUT;
  while ((WiFi.status() != WL_CONNECTED) and --timeoutClick > 0) {
    delay(CHECK_TIMEOUT);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to " + String(ssid.c_str()));
    Serial.println("IP " + WiFi.localIP().toString());
    pwm.add(ssid, password);
    pwm.promote(ssid);
    pwm.save();
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
  status = DISCONNECTED;
  delay(100);
  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP("Pipo", "pipo1234")) {
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
  bool wasConnected = status == CONNECTED;
  String previousSsid = WiFi.SSID();
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.mode(WIFI_MODE_APSTA);
  success = WiFi.softAP("Pipo", "pipo1234");
  if (wasConnected) {
    success &= connect(previousSsid, pwm.getPassword(previousSsid), false);
  }
  return success;
};

bool PipoWifi::STAMode() {
  if (WiFi.getMode() == WIFI_MODE_STA && status == CONNECTED) {
    return true;
  }
  bool wasConnected = status == CONNECTED;
  String previousSsid = WiFi.SSID();
  WiFi.disconnect(true, true);
  delay(100);
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
  String res;
  for (auto const& ssid : signals) {
    res += ssid.first;
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

PipoWifi wifi;