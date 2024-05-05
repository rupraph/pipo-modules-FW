// this will implement the methods from config.h


#include "config.h"



Config::Config() {
    
}

void Config::load_config_from_file(String filename) {
    current_config = json::parse(readFile(LittleFS,filename.c_str()));
}

void Config::save_config(String filename) {
    writeFile(LittleFS, filename.c_str(), current_config.dump().c_str());
}

json Config::get_config_for_key(string key) {
    return current_config.at(key);
}

// void Config::save_config_for_key(string key, json data) {
//     current_config[key] = data;
// }

void Config::gather_current_config(sensor& sensor,Engine& engine,bool debug) {
    current_config["sensor"] = sensor.get_config(debug);
    current_config["engine"] = engine.get_config(debug);
    if (debug) {
        Serial.println("gathered_config");
        Serial.println(current_config.dump(4).c_str());
        Serial.println("gathered_config_end");
    }
}

void Config::set_current_config(sensor& sensor,Engine& engine,bool debug) {
    sensor.set_config(current_config["sensor"]);
    engine.set_config(current_config["engine"]);
}