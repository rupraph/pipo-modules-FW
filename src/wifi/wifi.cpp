#include <wifi/wifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  preferences.begin("pipo-wifi", false);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.onEvent([&](WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
      scan();
      break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      connect();
      break;
    }
  });
  WiFi.begin();
};
void PipoWifi::scan() {
  int num = WiFi.scanNetworks();
  for (int i = 0; i < num; i++) {
    int rssiperc = WiFi.RSSI(i);
    ssids[std::string(WiFi.SSID(i).c_str())] = rssiperc;
  }
};
void PipoWifi::connect() {
  int success = 0;
  mode = CONNECTING;
  for (auto const &ssid : ssids) {
    try {
      String password = preferences.getString(ssid.first.c_str());
      // try to connect
      int status = WiFi.begin(ssid.first.c_str(), password.c_str());
      if (status == WL_CONNECTED) {
        success = 1;
        mode = CONNECTED;
        break;
      }
    } catch (const std::exception &e) {
      continue;
    }
  }
  if (success)
    return;
  // if no success, switch to AP mode
  mode = AP;
  WiFi.softAP("Pipo", "pipo1234");
};

bool PipoWifi::connect(std::string ssid, std::string password) {
  mode = CONNECTING;
  if (WiFi.mode(WIFI_AP)) {
    WiFi.softAPdisconnect(true);
  } else if (WiFi.mode(WIFI_STA)) {
    WiFi.disconnect();
  }
  int result = WiFi.begin(ssid.c_str(), password.c_str());
  if (result == WL_CONNECTED) {
    save(ssid, password);
    mode = AP;
    return true;
  }
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
  WiFi.softAP("Pipo", "pipo1234");
  mode = AP;
};
PipoWifi::PipoWifiMode PipoWifi::getMode() { return mode; };
