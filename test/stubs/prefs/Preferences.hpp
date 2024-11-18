#ifndef PIPOSTUBPREFERENCES_H
#define PIPOSTUBPREFERENCES_H
#include <Arduino.h>
#include <map>

class Preferences {

 public:
  std::map<std::string, std::string>* map;
  Preferences() {
    std::cout << "Preferences created" << std::endl;
    map = new std::map<std::string, std::string>();
  }
  void begin(const char* name, bool readOnly) {}
  void putString(String key, String value) {
    std::string k = std::string(key.c_str());
    std::string v = std::string(value.c_str());
    (*map)[k] = v;
  }
  String getString(String key, String defaultValue) {
    std::string k = std::string(key.c_str());
    if ((*map).find(k) == (*map).end())
      return defaultValue;
    return String((*map)[k].c_str());
  }
};

#endif  // PIPOSTUBPREFERENCES_H