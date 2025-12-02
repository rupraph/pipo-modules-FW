#include "config.h"

Config config;  // global config object so it can be accessed from anywhere

/// @brief Validate that the config has all required sections and they are not null/empty
/// @param config_doc The JSON document to validate
/// @return true if config is valid, false otherwise
bool Config::validate_config(JsonDocument& config_doc) {
  // Check for required top-level keys
  const char* required_keys[] = {"engine", "inputs", "general", "sensorconf"};

  for (const char* key : required_keys) {
    if (!config_doc.containsKey(key)) {
      log_e("Config validation failed: missing key '%s'", key);
      logs.writeError("Config validation: missing key '" + String(key) + "'");
      return false;
    }

    // Check if the key's value is null
    if (config_doc[key].isNull()) {
      log_e("Config validation failed: key '%s' is null", key);
      logs.writeError("Config validation: key '" + String(key) + "' is null");
      return false;
    }

    // For object types, check if they're empty
    if (config_doc[key].is<JsonObject>()) {
      JsonObject obj = config_doc[key].as<JsonObject>();
      if (obj.size() == 0) {
        log_e("Config validation failed: key '%s' is empty", key);
        logs.writeError("Config validation: key '" + String(key) +
                        "' is empty");
        return false;
      }
    }
  }

  // Additional validation: check that engine has its subsections
  if (config_doc["engine"].is<JsonObject>()) {
    JsonObject engine = config_doc["engine"].as<JsonObject>();
    const char* engine_keys[] = {"engine-hid", "engine-midi", "engine-osc"};

    for (const char* key : engine_keys) {
      if (!engine.containsKey(key) || engine[key].isNull()) {
        log_e("Config validation failed: engine missing or null key '%s'", key);
        logs.writeError("Config validation: engine missing/null key '" +
                        String(key) + "'");
        return false;
      }
    }
  }

  log_i("Config validation passed");
  return true;
}

/// @brief Restore a corrupted config by copying default.json
/// @param target_filename The filename to restore (without extension)
/// @return true if restore succeeded, false otherwise
bool Config::restore_from_default(String target_filename) {
  log_w("Attempting to restore config from default.json");
  logs.writeLog("Restoring config from default.json to " + target_filename);

  // Check if default.json exists
  if (!LittleFS.exists(config_model_path)) {
    log_e("CRITICAL: default.json not found, cannot restore");
    logs.writeError("CRITICAL: default.json not found");
    return false;
  }

  String target_path = get_path(target_filename);

  // Delete the corrupted file first
  if (LittleFS.exists(target_path.c_str())) {
    LittleFS.remove(target_path.c_str());
    log_i("Removed corrupted config: %s", target_path.c_str());
  }

  // Copy default.json to target (copyFile returns void, so we verify afterward)
  copyFile(LittleFS, config_model_path, target_path.c_str());

  // Verify the copy succeeded by checking if the file exists and has content
  if (!LittleFS.exists(target_path.c_str())) {
    log_e("Failed to restore config: file not created");
    logs.writeError("Failed to restore config: " + target_filename);
    return false;
  }

  File verifyFile = LittleFS.open(target_path.c_str(), FILE_READ);
  if (!verifyFile) {
    log_e("Failed to restore config: cannot open file");
    logs.writeError("Failed to restore config (cannot open): " +
                    target_filename);
    return false;
  }

  size_t fileSize = verifyFile.size();
  verifyFile.close();

  if (fileSize < 10) {
    log_e("Failed to restore config: file too small");
    logs.writeError("Failed to restore config (too small): " + target_filename);
    return false;
  }

  log_i("Successfully restored config from default.json");
  logs.writeLog("Config restored successfully: " + target_filename);
  return true;
}

bool Config::load_config(String filename, bool addJsonExtension = true) {
  this->filename = filename;
  String configPath = get_path(filename, addJsonExtension);
  log_i("load config: %s", configPath.c_str());

  try {
    if (DEBUG_HEAP)
      pipoDebugHeap("Config: load config");

    if (DEBUG_CONFIG) {
      log_d("config: before loading");
      serializeJsonPretty(current_config, Serial);
    }

    // Check if file exists
    if (!LittleFS.exists(configPath.c_str())) {
      log_e("Config file not found: %s", configPath.c_str());
      logs.writeError("Config file not found: " + configPath);
      return false;
    }

    // Check file size (basic sanity check)
    File f = LittleFS.open(configPath.c_str(), FILE_READ);
    if (!f) {
      log_e("Failed to open config file: %s", configPath.c_str());
      logs.writeError("Failed to open config file: " + configPath);
      return false;
    }
    size_t fileSize = f.size();
    f.close();

    if (fileSize < 10) {  // Too small to be a valid config
      log_e("Config file too small (likely corrupted): %d bytes", fileSize);
      logs.writeError("Config file too small: " + String(fileSize) + " bytes");
      return false;
    }

    current_config.clear();
    DeserializationError error =
        deserializeJson(current_config, readFile(LittleFS, configPath.c_str()));
    if (error) {
      log_e("deserializeJson() failed: %s", error.c_str());
      logs.writeError("Error loading config: " + String(error.c_str()));
      return false;
    }

    // NEW: Comprehensive validation to detect incomplete configs
    if (!validate_config(current_config)) {
      log_w(
          "Config validation failed, attempting to restore from default.json");
      logs.writeError("Config validation failed for: " + filename);

      // Try to restore from default
      if (restore_from_default(filename)) {
        // Reload the restored config
        log_i("Reloading restored config...");
        current_config.clear();
        error = deserializeJson(current_config,
                                readFile(LittleFS, configPath.c_str()));

        if (error) {
          log_e("Failed to reload restored config");
          logs.writeError("Failed to reload restored config");
          return false;
        }

        // Validate again
        if (!validate_config(current_config)) {
          log_e("Restored config still invalid");
          logs.writeError("Restored config validation failed");
          return false;
        }

        log_i("Config successfully restored and validated");
        logs.writeLog("Config restored and validated: " + filename);
      } else {
        log_e("Failed to restore config from default.json");
        return false;
      }
    }

    // Update general_config from loaded file
    general_config.clear();
    general_config = current_config["general"];

    if (DEBUG_CONFIG) {
      log_d("loaded config:");
      serializeJsonPretty(current_config, Serial);
    }
    logs.writeLog("load config: " + filename);
    if (DEBUG_HEAP)
      pipoDebugHeap("Config: end load config");

    return true;
  } catch (const std::exception& e) {
    log_e("error loading config: %s", e.what());
    logs.writeError("Exception loading config: " + String(e.what()));
    return false;
  }
}

/// @brief Clean up any orphaned .tmp files from previous crashes
void Config::cleanup_temp_files() {
  log_i("Cleaning up potential temp config files...");
  File root = LittleFS.open(configs_root);
  if (!root || !root.isDirectory()) {
    return;
  }

  File file = root.openNextFile();
  while (file) {
    String name = String(file.name());
    if (name.endsWith(temp_suffix)) {
      String fullPath = String(configs_root) + "/" + name;
      file.close();
      LittleFS.remove(fullPath.c_str());
      log_i("Cleaned up temp file: %s", fullPath.c_str());
      logs.writeLog("Cleaned up orphaned temp file: " + name);
      file = root.openNextFile();
    } else {
      file = root.openNextFile();
    }
  }
  root.close();
}

/// @brief Load the last config used, if it exists, otherwise load the default
/// config.
void Config::load_config() {
  // if no default config, create default
  if (!LittleFS.exists(get_path("Config-1").c_str())) {
    log_i("no default config, creating one");
    new_config("Config-1");
  }
  // if last config exists, load it
  if (LittleFS.exists(last_config_path)) {
    String name = String(readFile(LittleFS, last_config_path).c_str());
    if (LittleFS.exists(get_path(name).c_str())) {
      log_i("last config found: %s", name.c_str());
      bool success = load_config(name);
      if (!success) {
        // Config is corrupted, delete it and fallback to default
        log_w("Config corrupted, deleting: %s", name.c_str());
        logs.writeError("Deleting corrupted config: " + name);
        LittleFS.remove(get_path(name).c_str());

        // Load default config
        log_i("Falling back to default config");
        bool defaultSuccess = load_config("Config-1");
        if (!defaultSuccess) {
          log_e("CRITICAL: Default config is also corrupted!");
          logs.writeError("CRITICAL: Default config corrupted, recreating");
          // Recreate default from model
          new_config("Config-1");
          load_config("Config-1");
        }
      }
      return;
    }
    log_i("last config not found, loading default");
  }
  // if no last config, load default
  bool success = load_config("Config-1");
  if (!success) {
    log_e("CRITICAL: Default config corrupted, recreating");
    logs.writeError("CRITICAL: Default config corrupted, recreating");
    new_config("Config-1");
    load_config("Config-1");
  }
}

String Config::get_list() {
  File root = LittleFS.open(configs_root);
  if (!root || !root.isDirectory()) {
    log_e("failed to open config root");
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
  // Validate config before saving to prevent writing incomplete configs
  if (!validate_config(current_config)) {
    log_e("CRITICAL: Attempted to save invalid config, operation aborted!");
    logs.writeError("Save aborted: config validation failed for " + filename);
    return;
  }

  String finalPath = get_path(filename);
  String tempPath = finalPath + temp_suffix;

  log_i("save config (atomic): %s", finalPath.c_str());

  // Write to temporary file first
  File file = LittleFS.open(tempPath.c_str(), FILE_WRITE);
  if (!file) {
    log_e("failed to open temp file for writing");
    logs.writeError("Failed to open temp file for writing: " + filename);
    return;
  }

  size_t bytesWritten = serializeJson(current_config, file);
  file.close();

  if (bytesWritten == 0) {
    log_e("Failed to write to temp file (0 bytes written)");
    logs.writeError("Failed to write config to temp (0 bytes): " + filename);
    LittleFS.remove(tempPath.c_str());
    log_i("Removed corrupted temp file");
    return;
  }

  // Verify temp file before committing
  File verifyFile = LittleFS.open(tempPath.c_str(), FILE_READ);
  if (!verifyFile || verifyFile.size() != bytesWritten) {
    log_e("Temp file verification failed");
    logs.writeError("Temp file verification failed for: " + filename);
    if (verifyFile)
      verifyFile.close();
    LittleFS.remove(tempPath.c_str());
    return;
  }
  verifyFile.close();

  // Atomic rename: this is the critical moment
  // If power fails here, either old or new config exists (never partial)
  if (LittleFS.rename(tempPath.c_str(), finalPath.c_str())) {
    log_i("Config saved successfully (%d bytes)", bytesWritten);
    logs.writeLog("save config: " + filename);
  } else {
    log_e("Failed to rename temp file to final config");
    logs.writeError("Failed to commit config (rename failed): " + filename);
    LittleFS.remove(tempPath.c_str());
  }
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
      bool success = load_config(name.substring(0, name.length() - 5));
      if (!success) {
        log_w("Next config corrupted, falling back to default");
        logs.writeError("Next config corrupted: " + name);
        LittleFS.remove(get_path(name.substring(0, name.length() - 5)).c_str());
        load_config();
      }
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
      log_e("deserializeJson() failed: %s", error.c_str());
      logs.writeError("Error setting config: " + String(error.c_str()));
      return;
    }
    // current_config = config;
    log_i("config set");
    logs.writeLog("config set");
  } catch (const std::exception& e) {
    log_e("error setting current_config from a json object: %s", e.what());
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
  log_d("gatherconfig sensor");
  current_config["inputs"].clear();
  current_config["inputs"] = input_sensor.get_inputs_config();
  log_d("gatherconfig engine");
  current_config["engine"].clear();
  current_config["engine"] = engine.get_config();
  log_d("gatherconfig general");
  current_config["general"].clear();
  current_config["general"] = general_config;
  log_d("gatherconfig sensorconf");
  current_config["sensorconf"].clear();
  current_config["sensorconf"] = input_sensor.get_sensor_config();

  // Validate the gathered config
  if (!validate_config(current_config)) {
    log_w("WARNING: Gathered config is incomplete!");
    logs.writeError("Gathered config validation failed");

    // Log which sections are problematic
    if (current_config["inputs"].isNull() ||
        (current_config["inputs"].is<JsonObject>() &&
         current_config["inputs"].as<JsonObject>().size() == 0)) {
      log_w("  - inputs section is null or empty");
    }
    if (current_config["engine"].isNull() ||
        (current_config["engine"].is<JsonObject>() &&
         current_config["engine"].as<JsonObject>().size() == 0)) {
      log_w("  - engine section is null or empty");
    }
    if (current_config["general"].isNull() ||
        (current_config["general"].is<JsonObject>() &&
         current_config["general"].as<JsonObject>().size() == 0)) {
      log_w("  - general section is null or empty");
    }
    if (current_config["sensorconf"].isNull() ||
        (current_config["sensorconf"].is<JsonObject>() &&
         current_config["sensorconf"].as<JsonObject>().size() == 0)) {
      log_w("  - sensorconf section is null or empty");
    }
  } else {
    log_i("Gathered config validated successfully");
  }

  if (debug) {
    log_d("gathered_config");
    serializeJsonPretty(current_config, Serial);
    log_d("gathered_config_end");
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