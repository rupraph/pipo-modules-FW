#include "config.h"

Config config;  // global config object so it can be accessed from anywhere

void Config::load_config(String filename, bool addJsonExtension = true) {
  this->filename = filename;
  Serial.print("load config: ");
  Serial.println(get_path(filename, addJsonExtension).c_str());
  try {
#ifdef DEBUG_HEAP
    Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif

    if (DEBUG_CONFIG) {
      Serial.println("config: before loading:");
      Serial.println(current_config.dump(4).c_str());
    }

    current_config.clear();
    current_config = json::parse(
        readFile(LittleFS, get_path(filename, addJsonExtension).c_str()));

    if (DEBUG_CONFIG) {
      Serial.println("loaded config:");
      Serial.println(current_config.dump(4).c_str());
    }

#ifdef DEBUG_HEAP
    Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
    logs.writeLog("load config: " + filename);
  } catch (const std::exception& e) {
    Serial.println("error loading config");
    Serial.println(e.what());
  }
}

/// @brief Load the last config used, if it exists, otherwise load the default config.
void Config::load_config() {
  // if no default config, create default
  if (!LittleFS.exists(get_path("default").c_str())) {
    Serial.println("no default config, creating one");
    new_config("default");
  }
  // if last config exists, load it
  if (LittleFS.exists(last_config_path)) {
    String name = String(readFile(LittleFS, last_config_path).c_str());
    if (LittleFS.exists(get_path(name).c_str())) {
      Serial.println("last config found: " + name);
      load_config(name);
      return;
    }
    Serial.println(F("last config not found, loading default"));
  }
  // if no last config, load default
  load_config("default");
}

String Config::get_list() {
  File root = LittleFS.open(configs_root);
  if (!root || !root.isDirectory()) {
    throw std::runtime_error("failed to open configs root");
  }
  String list;
  File file = root.openNextFile();
  while (file) {
    String name = String(file.name());
    if (name.endsWith(".json")) {
      list += name.substring(0, name.length() - 5);
      file = root.openNextFile();
      if (file) {
        list += ",";
      }
    }
  }

  root.close();
  file.close();
  return list;
}

void Config::save() {
  save(filename);
}
void Config::save(String filename) {
  save(filename, current_config.dump().c_str());
}

void Config::save(String filename, String config) {
  logs.writeLog("save config: " + filename);
  Serial.println("save config: " + filename);
  writeFile(LittleFS, get_path(filename, false).c_str(), config.c_str());
}

void Config::delete_config(String filename) {
  LittleFS.remove(get_path(filename).c_str());
  logs.writeLog("delete config: " + filename);
  if (filename == this->filename) {
    File root = LittleFS.open(configs_root);
    File file = root.openNextFile();
    if (!file) {
      load_config();
      logs.writeLog("deleted last config, creating new default");
    } else {
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
  // should check if file already exists.
  copyFile(LittleFS, config_model_path, get_path(name).c_str());
  this->filename = name;
  logs.writeLog("new config: " + name);
}

json Config::get(string key) {
  return current_config.at(key);
}
json Config::get() {
  return current_config;
}

void Config::set(const json& config) {
  try {
    //Serial.println(config.dump().c_str());
    current_config.clear();
    current_config = config;
    logs.writeLog("config set");
  } catch (const std::exception& e) {
    Serial.println("error setting current_config from a json object");
    Serial.println(e.what());
    logs.writeError("Error setting config: " + String(e.what()));
  }
}

void Config::print() {
  Serial.println(current_config.dump(4).c_str());
}

void Config::gather(Sensor& sensor, Engine& engine, bool debug) {
  Serial.print("gatherconfig sensor");
  current_config["sensor"].clear();
  current_config["sensor"] = sensor.get_config(debug);
  Serial.print("gatherconfig engine");
  current_config["engine"].clear();
  current_config["engine"] = engine.get_config(debug);
  Serial.print("gatherconfig general");
  current_config["general"].clear();
  current_config["general"] = general_config;

  if (debug) {
    Serial.println("gathered_config");
    Serial.println(current_config.dump(4).c_str());
    Serial.println("gathered_config_end");
  }
}

//* @brief propagates the current config content to the sensor, engine, etc...
void Config::apply(Sensor& sensor, Engine& engine, OSC_handler& osc,
                   bool debug) {
  try {
    sensor.set_config(current_config["sensor"], DEBUG_CONFIG);
    engine.set_config(current_config["engine"], DEBUG_CONFIG);
    general_config.clear();
    general_config = current_config["general"];
    //log last config name
    writeFile(LittleFS, last_config_path, filename.c_str());
    /*TODO: improve: 
        either pass the json to apply and avoid passing cofig object
        to osc class, or follow the same config process than sensor 
        and engine instead of being in the general config... 
        */
    osc.set_config();

    Serial.println("config applied: " + filename);
    logs.writeLog("config applied: " + filename);
  } catch (const std::exception& e) {
    Serial.println("error applying config");
    Serial.println(e.what());
  }
}

String Config::get_path(String filename, bool addExtension) {
  if (addExtension) {
    return String(configs_root) + "/" + filename + ".json";
  }
  return String(configs_root) + "/" + filename;
}