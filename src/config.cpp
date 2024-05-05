// this will implement the methods from config.h


#include "config.h"



Config::Config() {
    
}

void Config::load_config(String filename) {
    current_config = json::parse(readFile(LittleFS,filename.c_str()));
    
}

void Config::save_config(String filename) {
    writeFile(LittleFS, filename.c_str(), current_config.dump().c_str());
}

json Config::get_config_for_key(string key) {
    return current_config.at(key);
}

void Config::save_config_for_key(string key, json data) {
    current_config[key] = data;
}

void Config::print_config() {

}