#include <wifi/pipowifi.h>
PipoWifi::PipoWifi(){};
void PipoWifi::setup() {
  Serial.println("Wifi setup");
  preferences.begin("pipo-wifi", false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  load();
  scan();
  connect();
};
void PipoWifi::scan() {
  int num = WiFi.scanNetworks(false, false, false, 500U);
  for (int i = 0; i < num; i++) {
    int rssiperc = WiFi.RSSI(i);
    // signals[WiFi.SSID(i)] = rssiperc;
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
  for (auto const& ssid : signals) {
    try {
      if (connect(ssid.first, passwords[ssid.first])) {
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
  return connect(ssid, passwords[ssid]);
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
    passwords[ssid] = password;
    save();
    mode = CONNECTED;
    return true;
  }
  Serial.println("Failed to connect to " + String(ssid.c_str()));
  Serial.println("Error: " + String(result));
  return false;
};

void PipoWifi::save() {
  String buffer = "";
  String indexes = "";
  for (auto const& pair : passwords) {
    indexes += pair.first.length();
    indexes += ' ';
    indexes += pair.second.length();
    indexes += ' ';
    buffer += pair.first;
    buffer += pair.second;
  }
  preferences.putString("indexes", indexes);
  preferences.putString("buffer", buffer);
};

void PipoWifi::load() {
  String indexes = preferences.getString("indexes", "");
  String buffer = preferences.getString("buffer", "");
  int n = 0;
  bool isPassword = false;
  int ssidStart = 0;
  int passwordStart = 0;
  int passwordEnd = 0;
  for (int i = 0; i < indexes.length(); i++) {
    if (indexes[i] != ',') {
      n *= 10;
      n += indexes[i] - '0';
      continue;
    }
    if (!isPassword) {
      isPassword = true;
      ssidStart = n;
    } else {
      if (!passwordStart) {
        passwordStart = n;
      } else {
        passwordEnd = n;
        passwords[buffer.substring(ssidStart, passwordEnd)] =
            buffer.substring(passwordEnd, n);
        passwordStart = 0;
        passwordEnd = 0;
        isPassword = false;
      }
    }
    n = 0;
  }
}
void PipoWifi::APMode() {
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP("Pipo", "pipo1234");
  mode = AP;
};
PipoWifi::PipoWifiMode PipoWifi::getMode() {
  return mode;
};

String PipoWifi::status() {
  String res;
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
String PipoWifi::availableNetworks() {
  String res;
  for (auto const& ssid : signals) {
    res += ssid.first;
    res += " ";
    res += String(ssid.second);
    res += strcmp(WiFi.SSID().c_str(), ssid.first.c_str()) == 0 ? " 1" : " 0";
    res += passwords[ssid.first].length() > 0 ? " 1" : " 0";
    res += "\n";
  }
  return res;
}