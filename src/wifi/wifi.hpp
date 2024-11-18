#ifndef PIPOWIFI2_H
#define PIPOWIFI2_H
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <iostream>
#include <map>
#include <string>
class PipoWifi {

 public:
  std::map<String, String> passwords;
  PipoWifi() {}
  void load() {}
};

#endif  // PIPOWIFI2_H
