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
    // if no default config, create default
    if (!LittleFS.exists(get_path("default").c_str())) {
        new_config("default");
    }
    // if last config exists, load it
    if (LittleFS.exists(last_config_path)) {
        String name = String(readFile(LittleFS, last_config_path).c_str());
        if(LittleFS.exists(get_path(name).c_str())){
            return load_config(name);
        }
    }
    // if no last config, load default
    load_config("default");
    print();
}
void Config::save() { save(filename); }
void Config::save(String filename) { save(filename, current_config.dump().c_str()); }
void Config::save(String filename, String config) {
    logs.writeLog("save config: " + filename);
    Serial.println("save config: " + filename);
    writeFile(LittleFS, get_path(filename).c_str(), config.c_str());
}

void Config::delete_config(String filename) {
    if (filename == "default") {
        return;
    }
    LittleFS.remove(get_path(filename).c_str());
    logs.writeLog("delete config: " + filename);
    if (filename == this->filename) {
        File root = LittleFS.open(configs_root);
        File file = root.openNextFile();
        if (!file) {
            load_config();
            logs.writeLog("deleted last config, creating new default");
        }else{
            String name = String(file.name());
            load_config(name.substring(0, name.length() - 5));
        }
        root.close();
        file.close();
    }
}
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
    // should check if file already exists. rewrtiing on same filename can cause corruption ? 
    writeFile(LittleFS, get_path(name).c_str(), input.c_str());
    logs.writeLog("new config: " + name);
}

json Config::get_configs() {
    File root = LittleFS.open(configs_root);
    File file = root.openNextFile();
    json res;
    std::map<std::string, std::string> map;
    // while loop should be avoided because blocking
    //  + this could return very big json object which might not fit the base ram and cause crash. return one file after the other to the client. 
    while (file) {
        res[file.name()] = readFile(LittleFS, get_path(file.name(), false).c_str());
        file = root.openNextFile();
    }
    root.close();
    file.close();
    return res;
}
json Config::get(string key) { return current_config.at(key); }
void Config::set(json config) {
    try {
        Serial.println(config.dump().c_str());
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
    Serial.print("gatherconfig general");
    current_config["general"] = general_config;

    if (debug) {
        Serial.println("gathered_config");
        Serial.println(current_config.dump(4).c_str());
        Serial.println("gathered_config_end");
    }
}
void Config::apply(Sensor& sensor, Engine& engine, OSC_handler& osc, bool debug) {
    sensor.set_config(current_config["sensor"]);
    engine.set_config(current_config["engine"]);
    general_config = current_config["general"];
    writeFile(LittleFS, last_config_path, filename.c_str());
     /*TODO: improve: 
     either pass the json to apply and avoid passing cofig object
      to osc class, or follow the same config process than sensor 
      and engine instead of being in the general config... 
     */ 
    osc.set_config();

    logs.writeLog("config applied: " + filename);
}
String Config::get_path(String filename, bool addExtension) {
    if (addExtension) {
        return String(configs_root) + "/" + filename + ".json";
    }
    return String(configs_root) + "/" + filename;
}