#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "fs_tools.h"
#include <ArduinoJson.h>
#include "utils/logs.h"
#include "utils/debug.h"
#include "../engine.h"
#include "../sensors/sensors.h"
#include "../osc/osc_handler.h"

using namespace std;

class Engine;
class OSC_handler;

class Config {
 public:
  Config() {
    general_config["Wifi_mode"] = "STA";
    general_config["OSC_ENA"] = true;
    general_config["OSC_PORT"] = 8000;
    general_config["OSC_IP"] = "0.0.0.0";
    general_config["MidiEnabled"] = true;
    general_config["HidEnabled"] = true;
    general_config["HidMode"] = 2;
    general_config["PipoName"] = "default";
  }
  String filename;              // raw config file name (no extension)
  JsonDocument current_config;  // stores all configs (gather)
  JsonDocument general_config;

  // load config from files into current_config
  void load_config(String filename, bool addJsonExtension = true);
  void load_config();
  // void shouldSave();
  // void saveIfNecessary();
  void set(const String& config);
  void setValue(char input[], int len);
  void setValues(char input[], int len);
  void save();
  void save(String filename);
  void save(String filename, String config);
  void delete_config(String filename);
  void rename(String old_name, String new_name);
  void new_config(String name);
  String get_list();

  JsonDocument get();            // return current_config
  JsonDocument get(string key);  // return current_config[key]
  // void save_for_key(string key, json data);
  void print();
  void gather(Engine& engine, bool debug = false);
  void apply(Engine& engine, OSC_handler& osc, bool debug = false);
  String get_path(String filename, bool add_extension = true);

 private:
  std::vector<std::string> split(const std::string& str, char delimiter);
  // bool _should_save = false;
  const char* last_config_path = "/last_config.txt";
  const char* config_model_path = "/default.json";
  const char* configs_root = "/configs";
  JsonDocument* tmp;
};

extern Config config;

#endif  //CONFIG_H