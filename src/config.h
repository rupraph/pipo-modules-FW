// This class will contain the configuration

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "fs_tools.h"
#include "json.hpp"
#include "engine.h"
#include "input_sensor.h"

using namespace std;
using json = nlohmann::json;

// the config file wil define the data structure and methods to save and load the configuration of the device
// the configuration will be saved in the preferences of the ESP32

class Config
{
    public:

        Config();
        json current_config;
        json test_config;
        void load_config_from_file(String filename);
        void load_config(string sensor_type);
        void save_config(String filename);
        void set_current_config(json config);
        json get_config_for_key(string key);
        // void save_config_for_key(string key, json data);
        void print_config();

        void gather_current_config(Sensor& sensor,Engine& engine,bool debug=false); 
        void apply_current_config(Sensor& sensor,Engine& engine,bool debug=false); 
};

#endif //CONFIG_H