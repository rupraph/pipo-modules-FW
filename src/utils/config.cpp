// this will implement the methods from config.h

#include "config.h"

// todo should be able to save different config and retrieve them (from webpage)
// should be able to report wherther config is succefully loade, saved,etc..

// todo. when changing sensor range for eg, this should trigger an update of
// miditranslator max ????
Config config;

void Config::load_config(String filename, bool addJsonExtension = true) {
  this->filename = filename;
  Serial.print("load config: ");
  Serial.println(get_path(filename, addJsonExtension).c_str());
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
  current_config.clear();
  current_config = json::parse(
      readFile(LittleFS, get_path(filename, addJsonExtension).c_str()));
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
  logs.writeLog("load config: " + filename);
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
  // should check if file already exists. rewrtiing on same filename can cause
  // corruption ?
  // std::string input = readFile(LittleFS, config_model_path);
  copyFile(LittleFS, config_model_path, get_path(name).c_str());
  this->filename = name;
  // writeFile(LittleFS, get_path(name).c_str(), input.c_str());
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
    // Serial.println(config.dump().c_str());
    current_config.clear();
    current_config = config;
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

void Config::setValue(String input) {
  Serial.println("set value: " + input);
  std::string path_value = input.c_str();
  // Split the input into path and value
  std::vector<std::string> path_and_value = split(path_value, ':');
  if (path_and_value.size() != 2) {
    throw std::invalid_argument("Input format should be 'path/to/key: value'");
  }

  std::string path = path_and_value[0];
  std::string value = path_and_value[1];

  // Split the path into individual keys
  std::vector<std::string> keys = split(path, '/');

  // Traverse the JSON object using the keys
  json* current = &current_config;
  for (size_t i = 0; i < keys.size() - 1; ++i) {
    Serial.print(keys[i].c_str());
    if (current->contains(keys[i])) {
      Serial.println(" exists ");
      current = &(*current)[keys[i]];
    } else {
      Serial.println(" does not exist ");

      // Create a new JSON object at this level if the key doesn't exist
      (*current)[keys[i]] = json::object();
      current = &(*current)[keys[i]];
    }
  }
  Serial.print("Before Assign ");
  Serial.println(current->dump().c_str());

  json* target = &(*current)[keys.back()];
  // Assign the value to the final key
  if (target->type() == json::value_t::string) {
    Serial.println("assigning string");
    (*current)[keys.back()] = value;
  } else if (target->type() == json::value_t::number_integer) {
    Serial.println("assigning integer");
    (*current)[keys.back()] = std::stoi(value);
  } else if (target->type() == json::value_t::number_float) {
    Serial.println("assigning float");
    (*current)[keys.back()] = std::stof(value);
  } else if (target->type() == json::value_t::boolean) {
    Serial.println("assigning boolean");
    (*current)[keys.back()] = value == "true";
  }
  Serial.print("After Assign ");
  Serial.println(current->dump().c_str());
}

void Config::print() {
  Serial.println(current_config.dump(4).c_str());
}

void Config::gather(Sensor& sensor, Engine& engine, bool debug = false) {
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
void Config::apply(Sensor& sensor, Engine& engine, OSC_handler& osc,
                   bool debug) {
  sensor.set_config(current_config["sensor"]);
  engine.set_config(current_config["engine"]);
  general_config.clear();
  general_config = current_config["general"];
  // log last config name
  // writeFile(LittleFS, last_config_path, filename.c_str());
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