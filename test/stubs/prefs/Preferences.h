#ifndef PIPOSTUBPREFERENCES_H
#define PIPOSTUBPREFERENCES_H
#include <Arduino.h>
#include <map>

class Preferences {

 public:
  std::map<std::string, std::string>* map;
  Preferences() { map = new std::map<std::string, std::string>(); }
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
  String getString(String key) { return getString(key, ""); }

  void putBool(String key, bool value) {
    std::string k = std::string(key.c_str());
    std::string v = value ? "1" : "0";
    (*map)[k] = v;
  }

  bool getBool(String key, bool defaultValue) {
    std::string k = std::string(key.c_str());
    if ((*map).find(k) == (*map).end())
      return defaultValue;
    return (*map)[k] == "1";
  }

  void putULong(String key, unsigned long value) {
    std::string k = std::string(key.c_str());
    char buf[32];
    sprintf(buf, "%lu", value);
    (*map)[k] = std::string(buf);
  }

  unsigned long getULong(String key, unsigned long defaultValue) {
    std::string k = std::string(key.c_str());
    if ((*map).find(k) == (*map).end())
      return defaultValue;
    return strtoul((*map)[k].c_str(), nullptr, 10);
  }
};

#endif  // PIPOSTUBPREFERENCES_H