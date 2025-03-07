#include "config.h"

Config config;  // global config object so it can be accessed from anywhere

void Config::load_config(String filename, bool addJsonExtension = true) {
  this->filename = filename;
  Serial.print("load config: ");
  Serial.println(get_path(filename, addJsonExtension).c_str());
  try {
    if (DEBUG_HEAP)
      pipoDebugHeap();

    if (DEBUG_CONFIG) {
      Serial.println("config: before loading:");
      serializeJsonPretty(current_config, Serial);
    }

    current_config.clear();
    DeserializationError error = deserializeJson(
        current_config,
        readFile(LittleFS, get_path(filename, addJsonExtension).c_str()));
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      logs.writeError("Error loading config: " + String(error.c_str()));
      return;
    }

    if (DEBUG_CONFIG) {
      Serial.println("loaded config:");
      serializeJsonPretty(current_config, Serial);
    }

    if (DEBUG_HEAP)
      pipoDebugHeap();
    logs.writeLog("load config: " + filename);
  } catch (const std::exception& e) {
    Serial.println("error loading config");
    Serial.println(e.what());
  }
}

/// @brief Load the last config used, if it exists, otherwise load the default
/// config.
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
    Serial.print("failed to open config root");
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
// void Config::shouldSave() {
//   _should_save = true;
// }
// void Config::saveIfNecessary() {
//   if (!_should_save) {
//     return;
//   }
//   _should_save = false;
//   save();
// }

void Config::save() {
  save(filename);
}
void Config::save(String filename) {
  //uses serialize method to write file
  Serial.print("save config: ");
  Serial.println(get_path(filename).c_str());

  File file = LittleFS.open(get_path(filename).c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("failed to open file for writing");
    return;
  }
  if (serializeJson(current_config, file) == 0) {
    Serial.println("Failed to write to file");
  } else {
    logs.writeLog("save config: " + filename);
  }
  file.close();
  // save(filename, current_config.dump().c_str());
}

// saving from a string
void Config::save(String filename, String config) {
  writeFile(LittleFS, get_path(filename).c_str(), config.c_str());
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

JsonDocument Config::get(string key) {
  return current_config[key];
}
JsonDocument Config::get() {
  return current_config;
}

void Config::set(const String& config) {
  try {
    // Serial.println(config.dump().c_str());
    current_config.clear();
    DeserializationError error = deserializeJson(current_config, config);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      logs.writeError("Error setting config: " + String(error.c_str()));
      return;
    }
    // current_config = config;
    Serial.println("config set");
    logs.writeLog("config set");
  } catch (const std::exception& e) {
    Serial.println("error setting current_config from a json object");
    Serial.println(e.what());
    logs.writeError("Error setting config: " + String(e.what()));
  }
}

std::vector<std::string> Config::split(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delimiter)) {
    tokens.push_back(token);
  }
  return tokens;
}

void Config::setValues(char input[], int len) {
  int start = 0;
  char c;
  for (int i = 0; i < len; i++) {
    c = input[i];
    if (c != '\n')
      continue;
    start = i + 1;
    setValue(input + start, i - start);
  }
  setValue(input + start, len - start);
}

void Config::setValue(char input[], int len) {
  // char value[64];
  // char key[64];
  // bool isValue = false;
  // int offset = 0;
  // int i = 0;
  // char c;
  // tmp = &current_config;

  // for (int i = 0; i < len; i++) {
  //   c = input[i];
  //   if (c == '\0') {
  //     break;
  //   }
  //   if (c == ':') {
  //     offset = 0;
  //     isValue = true;
  //     continue;
  //   }
  //   if (c == '/') {
  //     if (!tmp->contains(key)) {
  //       return;
  //     }
  //     tmp = &(*tmp)[key];
  //     offset = 0;
  //     isValue = c == ':';
  //     continue;
  //   }
  //   if (isValue) {
  //     value[offset++] = c;
  //     value[offset] = '\0';
  //   } else {
  //     key[offset++] = c;
  //     key[offset] = '\0';
  //   }
  // }
  // if (!tmp->contains(key)) {
  //   return;
  // }

  // Todo: to be updated with newer json lib
  // json* target = &(*tmp)[key];
  // // Assign the value to the final key
  // if (target->type() == json::value_t::string) {
  //   (*tmp)[key] = value;
  // } else if (target->type() == json::value_t::number_integer) {
  //   (*tmp)[key] = std::stoi(value);
  // } else if (target->type() == json::value_t::number_float) {
  //   (*tmp)[key] = std::stof(value);
  // } else if (target->type() == json::value_t::boolean) {
  //   (*tmp)[key] = value == "true";
  // }
  // target = nullptr;
}

void Config::print() {
  serializeJsonPretty(current_config, Serial);
}

//* @brief This gathers from all classes the config. does not save it.
void Config::gather(Engine& engine, bool debug) {
  Serial.println("gatherconfig sensor");
  current_config["inputs"].clear();
  current_config["inputs"] = input_sensor.get_inputs_config();
  Serial.println("gatherconfig engine");
  current_config["engine"].clear();
  current_config["engine"] = engine.get_config();
  Serial.println("gatherconfig general");
  current_config["general"].clear();
  current_config["general"] = general_config;
  Serial.println("gatherconfig sensorconf");
  current_config["sensorconf"].clear();
  current_config["sensorconf"] = input_sensor.get_sensor_config();

  if (debug) {
    Serial.println("gathered_config");
    serializeJsonPretty(current_config, Serial);
    Serial.println("gathered_config_end");
  }
}

//* @brief propagates the current config content to the sensor, engine, etc...
void Config::apply(Engine& engine, OSC_handler& osc, bool debug) {
  input_sensor.set_input_config(current_config["inputs"].as<JsonObject>(),
                                debug);
  input_sensor.set_sensor_config(current_config["sensorconf"].as<JsonObject>(),
                                 debug);
  engine.set_config(current_config["engine"].as<JsonObject>(), debug);
  general_config.clear();
  general_config = current_config["general"];
  /*TODO: improve:
  either pass the json to apply and avoid passing cofig object
   to osc class, or follow the same config process than sensor
   and engine instead of being in the general config...
  */
  osc.set_config();
  logs.writeLog("config applied: " + filename);
}
// catch (const std::exception& e) {
//   Serial.println("error applying config");
//   Serial.println(e.what());
// }
// }

String Config::get_path(String filename, bool addExtension) {
  if (addExtension) {
    return String(configs_root) + "/" + filename + ".json";
  }
  return String(configs_root) + "/" + filename;
}