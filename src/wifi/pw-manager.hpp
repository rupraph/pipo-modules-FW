#ifndef PIPOPWMANAGER_H
#define PIPOPWMANAGER_H
#include <Arduino.h>
#ifdef UNIT_TEST
#include <Preferences.hpp>
#else
#include <Preferences.h>
#endif
#include <map>
#include <string>

class PipoPWManager {

 public:
  PipoPWManager(){};

  void setup() {
    preferences.begin("pipo-wifi", false);
    load();
  };

  void save() {
    String buffer = "";
    String indexes = "";
    for (auto const& pair : passwords) {
      indexes += pair.first.length();
      indexes += ' ';
      indexes += pair.second.length();
      indexes += ' ';
      buffer += String(pair.first.c_str());
      buffer += String(pair.second.c_str());
    }
    preferences.putString("indexes", indexes);
    preferences.putString("buffer", buffer);
  };

  void load() {
    std::cout << "Loading preferences" << std::endl;
    String indexes = preferences.getString("indexes", "");
    String buffer = preferences.getString("buffer", "");
    // Serial.println("Indexes: " + indexes);
    int n = 0;
    bool isSSID = true;
    int ssidLen = 0;
    int pwdLen = 0;
    int offset = 0;
    int L = indexes.length();
    for (int i = 0; i < L; i++) {
      if (indexes[i] != ',') {
        n *= 10;
        n += indexes[i] - '0';
        continue;
      }
      if (isSSID) {
        isSSID = false;
        ssidLen = n;
        n = 0;
      } else {
        pwdLen = n;
        set(buffer, offset, ssidLen, pwdLen);
        offset += (ssidLen + pwdLen);
        std::cout << "offset: " << offset << std::endl;
        isSSID = true;
        ssidLen = 0;
        pwdLen = 0;
        n = 0;
      }
    }
    pwdLen = n;
    set(buffer, offset, ssidLen, pwdLen);
    for (auto const& pair : passwords) {
      std::cout << pair.first << "->" << pair.second << std::endl;
    }
  }

  void set(String buffer, int offset, int ssidLen, int pwdLen) {
    int pwdOffset = offset + ssidLen;
    String ssid = buffer.substring(offset, pwdOffset);
    String password = buffer.substring(pwdOffset, pwdOffset + pwdLen);
    passwords[std::string(ssid.c_str())] = std::string(password.c_str());
  }

  static const int MAX_NETWORKS = 10;
  Preferences preferences;
  std::map<std::string, std::string> passwords;
};

#endif  // PIPOPWMANAGER_H
