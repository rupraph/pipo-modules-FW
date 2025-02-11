#include <wifi/pipowifi.h>

void PipoWifi::onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Wifi connect event OSC/LED");
  osc.start();
  hwui.start_pulse(WIFI_LED, WIFI_STA_PULSE_TIME, 0, 255);
};

void PipoWifi::onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
  osc.stop();
  hwui.stop_pulse(WIFI_LED);
}

PipoWifi::PipoWifi() {};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  pwm.setup();
  WiFi.setAutoReconnect(true);
  // allow to connect to (WHY SO WEAK?) wep networks
  WiFi.setMinSecurity(WIFI_AUTH_WEP);
  WiFi.onEvent(onWifiConnect, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  // prevent from the Wifi to sleep: avoid latency in websockets
  WiFi.setSleep(false);
  scan();
  APSTAMode();

  //Todo add management of wifi disconnect, AP client connect, etc...
};

void PipoWifi::scan() {
  int num = WiFi.scanNetworks(false, false, false, 500U);
  for (int i = 0; i < num; i++) {
    int rssiperc = WiFi.RSSI(i);
    signals[WiFi.SSID(i)] = rssiperc;
  }
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
    // hwui.start_blink(WIFI_LED, 500, 0.2);
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
  success = WiFi.softAP("Pipo", "pipo1234");
  if (success) {
    Serial.println("APSTA mode started");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Failed to start APSTA mode");
  }
  // hwui.start_blink(WIFI_LED, 500, 0.2);
  return success & successConnect;
};

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
  String res;
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

PipoWifi wifi;