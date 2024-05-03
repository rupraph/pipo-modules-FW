// This class will contain the configuration

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>

using namespace std;

// the config file wil define the data structure and methods to save and load the configuration of the device
// the configuration will be saved in the preferences of the ESP32

class Config
{
    public:
        std::string current_config;
        Config();
        void load_config();
        void save_config();
        void print_config();
};


//     // hardware and connectivity settings
//     vector<string> usb_modes = {"usb_midi", "usb_serial"}; 
//     vector<string> ble_modes = {"ble_midi", "ble_serial"};
//     bool enable_ble;
//     bool enable_wifi;

//     // output protocol(s) selection
//     bool osc_enabled;
//     bool midi_enabled;

//     // settings for the chosen protocols
//     // midi settings

//     //sensor specific settings
//     bool enable_r_axis[3];
//     bool enable_t_axis[3];

//     //osc settings
//     String osc_ip;
//     int osc_port;

    

//     // methods
//     void save_config();
//     void load_config();
//     void print_config();
// };


#endif //CONFIG_H