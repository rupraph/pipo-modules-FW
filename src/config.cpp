// this will implement the methods from config.h


#include "config.h"

// todo should be able to save different config and retrieve them (from webpage)
// should be able to report wherther config is succefully loade, saved,etc..

//todo. when changing sensor range for eg, this should trigger an update of miditranslator max ???? 

Config::Config() {
    
}

void Config::load_config_from_file(String filename) {
    current_config = json::parse(readFile(LittleFS,filename.c_str()));
}

void Config::load_config(string sensor_type) {
    if (sensor_type == "motion") {
        load_config_from_file("/config/motion_config.json");
    }
    else if (sensor_type == "range") {
        load_config_from_file("/config/range_config.json");
    }
    else if (sensor_type == "analog") {
        load_config_from_file("/config/analog_config.json");
    }
}

void Config::save_config(String filename) {
    writeFile(LittleFS, filename.c_str(), current_config.dump().c_str());
}

json Config::get_config_for_key(string key) {
    return current_config.at(key);
}

void Config::set_current_config(json config) {
    try
    {
        current_config = config;

    }
    catch(const std::exception& e)
    {
        Serial.println("error setting current_config from a json object") ;
        Serial.println(e.what());
    }
    
    
}

void Config::print_config() {
    Serial.println(current_config.dump(4).c_str());
}

// void Config::save_config_for_key(string key, json data) {
//     current_config[key] = data;
// }

void Config::gather_current_config(Sensor& sensor,Engine& engine,bool debug) {
    Serial.print("gatherconfig sensor");
    current_config["sensor"] = sensor.get_config(debug);
    Serial.print("gatherconfig engine");
    current_config["engine"] = engine.get_config(debug);
    if (debug) {
        Serial.println("gathered_config");
        Serial.println(current_config.dump(4).c_str());
        Serial.println("gathered_config_end");
    }
}

//sensor& sensor,
void Config::apply_current_config(Sensor& sensor,Engine& engine,bool debug) {
    sensor.set_config(current_config["sensor"]);
    engine.set_config(current_config["engine"]);
}