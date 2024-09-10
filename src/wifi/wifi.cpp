#include <wifi/wifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  preferences.begin("pipo-wifi", false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  scan();
  connect();
};
void PipoWifi::scan() {
  int num = WiFi.scanNetworks(false, false, false, 500U);
  for (int i = 0; i < num; i++) {
    int rssiperc = WiFi.RSSI(i);
    ssids[std::string(WiFi.SSID(i).c_str())] = rssiperc;
    Serial.print("Network: ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" RSSI: ");
    Serial.println(rssiperc);
  }
  Serial.println("Finished scan");
};
void PipoWifi::connect() {
  Serial.println("Connect...");
  mode = CONNECTING;
  for (auto const &ssid : ssids) {
    try {
      std::string password =
          std::string(preferences.getString(ssid.first.c_str()).c_str());
      // try to connect
      if (connect(ssid.first, password)) {
        return;
      }
    } catch (const std::exception &e) {
      continue;
    }
  }
  // if no success, switch to AP mode
  Serial.println("No one to connect to, switching to AP mode");
  APMode();
};

bool PipoWifi::connect(std::string ssid) {
  std::string password =
      std::string(preferences.getString(ssid.c_str()).c_str());
  return connect(ssid, password);
}

bool PipoWifi::connect(std::string ssid, std::string password) {
  Serial.print("Connecting to " + String(ssid.c_str()));
  Serial.println(" with password " + String(password.c_str()));
  mode = CONNECTING;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  int result = WiFi.begin(ssid.c_str(), password.c_str());
  uint8_t timeoutClick = CONNECT_TIMEOUT / CHECK_TIMEOUT;
  while ((WiFi.status() != WL_CONNECTED) and --timeoutClick > 0) {
    delay(CHECK_TIMEOUT);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to " + String(ssid.c_str()));
    save(ssid, password);
    mode = CONNECTED;
    return true;
  }
  Serial.println("Failed to connect to " + String(ssid.c_str()));
  Serial.println("Error: " + String(result));
  return false;
};

void PipoWifi::save(std::string ssid, std::string password) {
  if (preferences.getString(ssid.c_str()).length() == 0) {
    preferences.putString(ssid.c_str(), password.c_str());
    return;
  }
  int num = preferences.getInt("num", -1);
  if (num < 5) {
    num++;
  }
  preferences.putInt("num", num);
  preferences.putString(ssid.c_str(), password.c_str());
  preferences.putString(String(num).c_str(), ssid.c_str());
};

void PipoWifi::APMode() {
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP("Pipo", "pipo1234");
  mode = AP;
};
PipoWifi::PipoWifiMode PipoWifi::getMode() { return mode; };

std::string PipoWifi::status() {
  std::string res;
  switch (mode) {
  case CONNECTING:
    res = "CONNECTING";
    break;
  case AP:
    res = "AP";
    break;
  case CONNECTED:
    res = "CONNECTED";
    res += " IP: ";
    res += WiFi.localIP().toString().c_str();
    res += " SSID: ";
    res += WiFi.SSID().c_str();
    break;
  }
  return res;
}
std::string PipoWifi::availableNetworks() {
  std::string res;
  for (auto const &ssid : ssids) {
    res += ssid.first;
    res += " ";
    res += std::to_string(ssid.second);
    res += strcmp(WiFi.SSID().c_str(), ssid.first.c_str()) == 0 ? " 1" : " 0";
    res += preferences.getString(ssid.first.c_str()).length() > 0 ? " 1" : " 0";
    res += "\n";
  }
  return res;
}