#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "fs_tools.h"
#include "utils/json.hpp"
#include "utils/logs.h"
#include "../engine.h"
#include "sensor/input_sensor.h"
#include "../osc_handler.h"

using namespace std;
using json = nlohmann::json;

class Engine;
class OSC_handler;

class Config {
 public:
  Config() {
    general_config = {
        {"Wifi_mode", "STA"},
        {"OSC_ENA", true},
        {"OSC_PORT", 8000},
        {"OSC_IP", "0.0.0.0"},
    };
  }
  String filename;  // raw config file name (no extension)
  json current_config;
  json general_config = {};

  // load config from files into current_config
  void load_config(String filename, bool addJsonExtension = true);
  void load_config();

  void set(const json& config);
  void setValue(String input);
  void save();
  void save(String filename);
  void save(String filename, String config);
  void delete_config(String filename);
  void rename(String old_name, String new_name);
  void new_config(String name);
  String get_list();

  json get();            // return current_config
  json get(string key);  // return current_config[key]
  // void save_for_key(string key, json data);
  void print();
  void gather(Sensor& sensor, Engine& engine, bool debug = false);
  void apply(Sensor& sensor, Engine& engine, OSC_handler& osc,
             bool debug = false);
  String get_path(String filename, bool add_extension = true);

 private:
  std::vector<std::string> split(const std::string& str, char delimiter);

  const char* last_config_path = "/last_config.txt";
  const char* config_model_path = "/default.json";
  const char* configs_root = "/configs";
};

extern Config config;

#endif  //CONFIG_H