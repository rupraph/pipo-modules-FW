// this will implement the methods from config.h

#include "config.h"

// todo should be able to save different config and retrieve them (from webpage)
// should be able to report wherther config is succefully loade, saved,etc..

// todo. when changing sensor range for eg, this should trigger an update of miditranslator max ????
Config config;

void Config::load_config(String filename) { current_config = json::parse(readFile(LittleFS, filename.c_str())); }

void Config::load_config() {
    String filename = "/config/";
#if defined(PIPO_MOTION)
    filename += "motion_config.json";
#elif defined(PIPO_RANGE)
    filename += "range_config.json";
#elif defined(PIPO_ANALOG)
    filename += "analog_config.json";
#endif
    load_config(filename);
}
void Config::save() {
    String filename = "/config/";
#if defined(PIPO_MOTION)
    filename += "motion_config.json";
#elif defined(PIPO_RANGE)
    filename += "range_config.json";
#elif defined(PIPO_ANALOG)
    filename += "analog_config.json";
#endif
    save(filename);
}
void Config::save(String filename) { writeFile(LittleFS, filename.c_str(), current_config.dump().c_str()); }

json Config::get(string key) { return current_config.at(key); }

void Config::set(json config) {
    try {
        current_config = config;
        logs.writeLog("config set");
    } catch (const std::exception& e) {
        Serial.println("error setting current_config from a json object");
        Serial.println(e.what());
        logs.writeError("Error setting config: " + String(e.what()));
    }
}

json Config::get() {
    return current_config;
}
void Config::print() { Serial.println(current_config.dump(4).c_str()); }

// void Config::save_for_key(string key, json data) {
//     current_config[key] = data;
// }

void Config::gather(Sensor& sensor, Engine& engine, bool debug) {
    Serial.print("gatherconfig sensor");
    current_config["sensor"] = sensor.get_config(debug);
    Serial.print("gatherconfig engine");
    current_config["engine"] = engine.get_config(debug);
    Serial.print("gatherconfig general");
    current_config["general"] = general_config;

    if (debug) {
        Serial.println("gathered_config");
        Serial.println(current_config.dump(4).c_str());
        Serial.println("gathered_config_end");
    }
}

// sensor& sensor,
void Config::apply(Sensor& sensor, Engine& engine, OSC_handler& osc, bool debug) {
    sensor.set_config(current_config["sensor"]);
    engine.set_config(current_config["engine"]);
    
    general_config = current_config["general"];
    Serial.println("general_config");
    osc.set_config(); // could likely be improved. either pass the json to apply and avoid passing cofig object to osc class, or follow the same config process than sensor and engine instead of being in the general config... 

    logs.writeLog("config apply");

}