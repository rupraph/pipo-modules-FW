// This class will contain the configuration

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "../engine.h"
#include "../osc_handler.h"
#include "fs_tools.h"
#include "sensor/input_sensor.h"
#include "utils/json.hpp"
#include "utils/logs.h"

using namespace std;
using json = nlohmann::json;

// the config file wil define the data structure and methods to save and load the configuration of the device
// the configuration will be saved in the preferences of the ESP32

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
  //json res;
  //json test_config;
  // only config element not comming from external classes.
  // placed here for now.
  json general_config = {
      // {"Wifi_mode", "STA"}, //can be AP, STA, OR AP_STA
      // {"OSC_ENA", true},
      // {"OSC_PORT", 5000},
      // {"OSC_IP", IPAddress(0,0,0,0)},
  };

  // load config from files into current_config
  void load_config(String filename, bool addJsonExtension = true);
  void load_config();

  // set current_config from a json object
  void set(const json& config);

  //load all config into

  //void load_all_configs();

  void save();
  void save(String filename);
  void save(String filename, String config);

  void delete_config(String filename);

  void rename(String old_name, String new_name);
  void new_config(String name);

  //json get_config_from_file(String filename);
  String get_list();
  //json get_configs();

  json get();            // return current_config
  json get(string key);  // return current_config[key]

  // void save_for_key(string key, json data);
  void print();
  void gather(Sensor& sensor, Engine& engine, bool debug = false);
  void apply(Sensor& sensor, Engine& engine, OSC_handler& osc,
             bool debug = false);
  String get_path(String filename, bool add_extension = true);

 private:
  const char* last_config_path = "/last_config.txt";
  const char* config_model_path = "/default.json";
  const char* configs_root = "/configs";
};

extern Config config;

#endif  //CONFIG_H