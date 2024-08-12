// this will implement the methods from config.h

#include "config.h"

// todo should be able to save different config and retrieve them (from webpage)
// should be able to report wherther config is succefully loade, saved,etc..

// todo. when changing sensor range for eg, this should trigger an update of miditranslator max ????
Config config;

void Config::load_config(String filename) {
    this->filename = filename;
    current_config = json::parse(readFile(LittleFS, get_path(filename).c_str()));
    logs.writeLog("load config: " + filename);

}

void Config::load_config() {
    if (!LittleFS.exists(get_path("default").c_str())) {
        new_config("default");
    }
    if (LittleFS.exists(last_config_path)) {
        return load_config(String(readFile(LittleFS, last_config_path).c_str()));
    }
    load_config("default");
}
void Config::save() { save(filename); }
void Config::save(String filename) { save(filename, current_config.dump().c_str()); }
void Config::save(String filename, String config) {
    logs.writeLog("save config: " + filename);
    writeFile(LittleFS, get_path(filename).c_str(), config.c_str()); 
}

void Config::delete_config(String filename) { LittleFS.remove(get_path(filename).c_str()); }
void Config::rename(String old_name, String new_name) {
    if (!LittleFS.exists(get_path(old_name).c_str())) {
        return;
    }
    LittleFS.rename(get_path(old_name).c_str(), get_path(new_name).c_str());
    logs.writeLog("rename config: " + old_name + " to " + new_name);
    if (old_name == filename) {
        filename = new_name;
        writeFile(LittleFS, last_config_path, filename.c_str());
    }
}
void Config::new_config(String name) {
    std::string input = readFile(LittleFS, config_model_path);
    writeFile(LittleFS, get_path(name).c_str(), input.c_str());
    logs.writeLog("new config: " + name);
}
json Config::get_configs() {
    File root = LittleFS.open(configs_root);
    File file = root.openNextFile();
    json res;
    std::map<std::string, std::string> map;
    while (file) {
        res[file.name()] = readFile(LittleFS, get_path(file.name(), false).c_str());
        file = root.openNextFile();
    }
    return res;
}
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
json Config::get() { return current_config; }
void Config::print() { Serial.println(current_config.dump(4).c_str()); }
void Config::gather(Sensor& sensor, Engine& engine, bool debug) {
    Serial.print("gatherconfig sensor");
    current_config["sensor"] = sensor.get_config(debug);
    Serial.print("gatherconfig engine");
    current_config["engine"] = engine.get_config(debug);
    current_config["general"] = general_config;

    if (debug) {
        Serial.println("gathered_config");
        Serial.println(current_config.dump(4).c_str());
        Serial.println("gathered_config_end");
    }
}
void Config::apply(Sensor& sensor, Engine& engine, bool debug) {
    sensor.set_config(current_config["sensor"]);
    engine.set_config(current_config["engine"]);
    general_config = current_config["general"];
    writeFile(LittleFS, last_config_path, filename.c_str());
    logs.writeLog("config applied: " + filename);
}
String Config::get_path(String filename, bool addExtension) {
    if (addExtension) {
        return String(configs_root) + "/" + filename + ".json";
    }
    return String(configs_root) + "/" + filename;
}