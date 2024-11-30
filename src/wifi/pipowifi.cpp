#include <wifi/pipowifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  pwm.setup();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  scan();
  connect();
};
void PipoWifi::scan() {
  int num = WiFi.scanNetworks(false, false, false, 300U);
  Serial.println("scan done");
  for (int i = 0; i < num; i++) {
    int rssiperc = WiFi.RSSI(i);
    signals[WiFi.SSID(i)] = rssiperc;
  }
};
void PipoWifi::connect() {
  Serial.println("Connect...");
  mode = CONNECTING;
  for (auto const& ssid : signals) {
    try {
      if (connect(ssid.first)) {
        return;
      }
    } catch (const std::exception& e) {
      continue;
    }
  }
  // if no success, switch to AP mode
  Serial.println("No one to connect to, switching to AP mode");
  APMode();
};

bool PipoWifi::connect(String ssid) {
  if (!pwm.hasSSID(ssid)) {
    return false;
  }
  return connect(ssid, pwm.getPassword(ssid));
}

bool PipoWifi::connect(String ssid, String password) {
  Serial.print("Connecting to " + ssid);
  Serial.println(" with password " + String(password));
  mode = CONNECTING;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  int result = WiFi.begin(ssid, password);
  uint8_t timeoutClick = CONNECT_TIMEOUT / CHECK_TIMEOUT;
  while ((WiFi.status() != WL_CONNECTED) and --timeoutClick > 0) {
    delay(CHECK_TIMEOUT);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to " + String(ssid.c_str()));
    Serial.println("IP " + WiFi.localIP().toString());
    pwm.add(ssid, password);
    pwm.save();
    mode = CONNECTED;
    return true;
  }
  Serial.println("Failed to connect to " + String(ssid.c_str()));
  Serial.println("Error: " + String(result));
  return false;
};

void PipoWifi::APMode() {
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP("Pipo", "pipo1234");
  mode = AP;
};
PipoWifi::PipoWifiMode PipoWifi::getMode() {
  return mode;
};

String PipoWifi::status() {
  String res = "";
  switch (wifi.getMode()) {
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

  switch (mode) {
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
    res += strcmp(WiFi.SSID().c_str(), ssid.first.c_str()) == 0 ? " 1" : " 0";
    res += pwm.hasSSID(ssid.first) ? " 1" : " 0";
    res += "\n";
  }
  return res;
}

String PipoWifi::ssid() {
  return WiFi.SSID();
}

PipoWifi wifi;