// This class will contain the configuration

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
#include "engine.h"
#include "sensor/input_sensor.h"

using namespace std;
using json = nlohmann::json;

// the config file wil define the data structure and methods to save and load the configuration of the device
// the configuration will be saved in the preferences of the ESP32

class Config
{
    public:

        Config(){
                general_config = {
                    {"Wifi_mode", "STA"},
        };
        }
        json current_config;
        json test_config;


        // only config element not comming from external classes. 
        // placed here for now.
        json general_config= {
        //{"Wifi_mode", "STA"}, //can be AP, STA, OR AP_STA
        };


        void load_config(String filename);
        void load_config();
        void save(String filename);
        void save();
        void set(json config);
        json get();
        json get(string key);
        // void save_for_key(string key, json data);
        void print();

        void gather(Sensor& sensor,Engine& engine,bool debug=false); 
        void apply(Sensor& sensor,Engine& engine,bool debug=false); 
};

extern Config config;

#endif //CONFIG_H