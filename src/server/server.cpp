#include "server/server.h"

PipoServer server;

// Decode base64 string
String base64Decode(const String& encoded) {
  const char* base64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String decoded = "";
  std::vector<int> T(256, -1);

  for (int i = 0; i < 64; i++)
    T[base64_chars[i]] = i;

  int val = 0, valb = -8;
  for (unsigned char c : encoded) {
    if (T[c] == -1)
      break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      decoded += char((val >> valb) & 0xFF);
      valb -= 8;
    }
  }
  return decoded;
}

// XOR decode password using MAC address as key
String decodePassword(const String& encoded, const String& mac) {
  if (encoded.isEmpty())
    return "";

  // Base64 decode first
  String decoded = base64Decode(encoded);

  // Remove colons from MAC to get consistent key
  String key = mac;
  key.replace(":", "");

  String password = "";
  for (size_t i = 0; i < decoded.length(); i++) {
    char charCode = decoded[i] ^ key[i % key.length()];
    password += charCode;
  }

  return password;
}

void PipoServer::setup() {
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "DELETE, POST, GET, OPTIONS");
  DefaultHeaders::Instance().addHeader(
      "Access-Control-Allow-Headers",
      "Origin, X-Requested-With, Content-Type, Accept");
  string mdns_name =
      string("pipo-") + config.general_config["PipoName"].as<string>();
  if (!MDNS.begin(
          mdns_name.c_str())) {  // Start the mDNS responder for esp.local
    log_e("Error setting up MDNS responder!");
  } else {
    log_i("mDNS responder started: %s", mdns_name.c_str());
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
  log_i("Starting HTTP server");
  //Todo: check lib exemple. can be improved
  fileServer = new PipoFileServer("/", LittleFS, "/webpage");

  presets.setup();
  setup_requests();
  // server.serveStatic("/", LittleFS, "/webpage/").setDefaultFile("index.html");
  // captivePortal.start(&server);
  // Add a custom 404 handler
  server.onNotFound([&](AsyncWebServerRequest* request) {
    log_w("File not found: %s", request->url().c_str());
    request->send(404, "text/plain", "File Not Found");
  });

  pipoSocket.start(&ws);
  server.addHandler(&ws);
  pipoSocket.setup();
  server.begin();
  is_running = true;

  if (DEBUG_HEAP)
    pipoDebugHeapFull("End server setup");
}
void PipoServer::pause() {
  pipoSocket.pause();
  is_running = false;
}
void PipoServer::resume() {
  pipoSocket.resume();
  is_running = true;
}
bool PipoServer::isRunning() {
  return is_running;
}
void PipoServer::setup_requests() {
  server.on("/info", HTTP_GET, [&](AsyncWebServerRequest* request) {
    log_d("HTTP request: /info");
    if (DEBUG_HEAP)
      pipoDebugHeap("info request");
#ifdef PIPO_FW_VERSION
    const char* version = stringify(PIPO_FW_VERSION);
#else
    const char* version = "unknown";
#endif
    String info = "{";
    info += "\"name\":\"unnamed Pipo\",";
    info += "\"version\":\"";
    info += version;
    info += "\",";
    info += "\"type\":\"";
    info += PIPO_TYPE;
    info += "\",";
    info += "\"ip\":\"";
    info += WiFi.localIP().toString();
    info += "\",";
    info += "\"mac\":\"";
    info += WiFi.macAddress();
    info += "\"}";
    if (DEBUG_HEAP)
      pipoDebugHeap("end info request");
    return request->send(200, "text/json", info.c_str());
  });

  server.on("/generate_204", HTTP_GET,
            [](AsyncWebServerRequest* request) { request->redirect("/"); });

  server.on("/hotspot-detect.html", HTTP_GET,
            [](AsyncWebServerRequest* request) { request->redirect("/"); });

  server.on("/success.html", HTTP_GET,
            [](AsyncWebServerRequest* request) { request->redirect("/"); });

  server.on("/success.txt", HTTP_GET,
            [](AsyncWebServerRequest* request) { request->redirect("/"); });

  // recevies and apply config
  server.on("/config", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("config")) {
      return request->send(400, "text/plain", "No config received");
    }
    try {
      if (DEBUG_HEAP)
        pipoDebugHeapFull("config request");
      config.set(request->getParam("config")->value());

      config.apply(engine, osc, DEBUG_CONFIG);
      config.save();
      if (DEBUG_HEAP)
        pipoDebugHeap("end config request");
      return request->send(200, "text/plain", "Config set");
    } catch (std::exception e) {
      return request->send(500, "text/plain",
                           "Error while setting config: " + String(e.what()));
    }
  });

  // sends config-list and config file based on provided filename
  server.on("/configs", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("name")) {
      if (DEBUG_HEAP)
        pipoDebugHeap("request: retrieving config list");
      String list = config.get_list();
      if (DEBUG_HEAP)
        pipoDebugHeap("retrived config list");
      return request->send(200, "text/plain", list);
    }
    try {
      String name = request->getParam("name")->value();
      if (DEBUG_HEAP)
        pipoDebugHeap("request: start send config file ");
      return request->send(LittleFS, config.get_path(name), "application/json");
    } catch (const std::exception e) {
      return request->send(500, "text/plain",
                           "Error loading config: " + String(e.what()));
    }
  });

  // sends active config filename
  server.on("/config-active", HTTP_GET, [&](AsyncWebServerRequest* request) {
    return request->send(200, "text/plain", config.filename.c_str());
  });

  // set active config
  server.on("/active-config", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("name")) {
      return request->send(400, "text/plain", "Error: no name parameter");
    }
    try {
      if (DEBUG_HEAP)
        pipoDebugHeap("request: set active config");
      bool success =
          config.load_config(request->getParam("name")->value().c_str(), true);
      if (!success) {
        log_w("Failed to load config, using current config");
        return request->send(500, "text/plain",
                             "Error: Config file corrupted or invalid");
      }
      config.apply(engine, osc, DEBUG_CONFIG);
      if (DEBUG_HEAP)
        pipoDebugHeap("end set ctive config");
      return request->send(200, "text/plain", "Active config set");
    } catch (const std::exception e) {
      log_e("error loading config: %s", e.what());
      return request->send(500, "text/plain",
                           "Error loading config: " + String(e.what()));
    }
  });

  server.on("/config-delete", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("name")) {
      return request->send(400, "text/plain", "Error: no name parameter");
    }
    try {
      config.delete_config(request->getParam("name")->value());
      config.apply(engine, osc, DEBUG_CONFIG);
      return request->send(200, "text/plain", "Config deleted");
    } catch (const std::exception e) {
      return request->send(500, "text/plain",
                           "Error deleting config: " + String(e.what()));
    }
  });
  server.on("/config-new", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("name")) {
      return request->send(400, "text/plain", "Error: no name parameter");
    }
    try {
      config.new_config(request->getParam("name")->value());
      return request->send(200, "text/plain", "Config created");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error creating config: " + String(e.what()));
    }
  });
  server.on("/config-copy", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("name") || !request->hasParam("config")) {
      return request->send(400, "text/plain",
                           "Error: no name or config parameter");
    }
    try {
      config.save(request->getParam("name")->value(),
                  request->getParam("config")->value());
      return request->send(200, "text/plain", "Config copied");
    } catch (const std::exception e) {
      return request->send(500, "text/plain",
                           "Error copying config: " + String(e.what()));
    }
  });
  server.on("/config-rename", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("oldname") || !request->hasParam("newname")) {
      return request->send(400, "text/plain",
                           "Error: no old or new name parameter");
    }
    try {
      config.rename(request->getParam("oldname")->value(),
                    request->getParam("newname")->value());
      return request->send(200, "text/plain", "Config renamed");
    } catch (const std::exception e) {
      return request->send(500, "text/plain",
                           "Error renaming config: " + String(e.what()));
    }
  });

  server.on(
      "/save", HTTP_POST,
      [&](AsyncWebServerRequest* request) {
        return request->send(200, "text/plain", "Config sending");
      },
      [&](AsyncWebServerRequest* request, String filename, size_t index,
          uint8_t* data, size_t len, bool final) {
        try {
          if (index == 0) {
            // This is the start of the file upload
            received_configData.clear();
            if (DEBUG_HEAP)
              pipoDebugHeap("Request: receive config data start");
          }
          received_configData.append((char*)data, len);

          if (final) {
            // This is the end of the file upload
            // Here I am doing save first then load. so parsing happen with load function.
            // this avoids parsing in here and trying to pass the json to config.set().
            // after solving other issues, not sure if this has any value after all.

            if (DEBUG_HEAP)
              pipoDebugHeap("Request: config data received");
            config.save(config.filename, received_configData.c_str());
            bool success = config.load_config(config.filename);
            if (!success) {
              received_configData.clear();
              if (DEBUG_HEAP)
                pipoDebugHeap("Request: config load failed");
              return request->send(500, "text/plain",
                                   "Error: Uploaded config is invalid");
            }
            config.apply(engine, osc, DEBUG_CONFIG);
            received_configData.clear();
            if (DEBUG_HEAP)
              pipoDebugHeapFull("Request: config saved");
            return request->send(200, "text/plain", "Config saved");
          }
        } catch (const std::exception& e) {
          log_e("error saving config: %s", e.what());
          return request->send(500, "text/plain",
                               "Error saving config: " + String(e.what()));
        }
      });

  server.on("/reboot", HTTP_GET, [&](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Rebooting");
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP.restart();
  });

  server.on("/wifi-mode", HTTP_POST, [&](AsyncWebServerRequest* request) {
    log_d("POST wifi-mode");
    if (!request->hasParam("mode")) {
      log_w("no mode");

      return request->send(400, "text/plain", "Error: no mode parameter");
    }
    String mode = request->getParam("mode")->value();
    log_d("mode: %s", mode.c_str());
    if (mode != "AP" && mode != "STA" && mode != "APSTA") {
      return request->send(400, "text/plain", "Error: invalid mode");
    }
    request->send(200, "text/plain", "Try to switch to mode " + mode);
    vTaskDelay(pdMS_TO_TICKS(100));
    pause();
    config.general_config["Wifi_mode"] = mode;
    log_i("Setting mode: %s", mode.c_str());
    if (mode == "AP") {
      wifi.setMode(WIFI_AP);
    } else if (mode == "STA") {
      wifi.setMode(WIFI_STA);
    } else {
      wifi.setMode(WIFI_AP_STA);
    }
  });

  server.on("/wifi-connect", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("ssid")) {
      return request->send(400, "text/plain", "Error: no ssid parameter");
    }

    String ssid = request->getParam("ssid")->value();
    String password = "";
    bool isEncoded = false;

    if (request->hasParam("password")) {
      password = request->getParam("password")->value();
    }

    if (request->hasParam("encoded")) {
      isEncoded = request->getParam("encoded")->value() == "true";
    }

    // Decode password if it was encoded
    if (isEncoded && !password.isEmpty()) {
      String mac = WiFi.macAddress();
      password = decodePassword(password, mac);
      log_d("Password decoded using MAC");
    }

    request->send(200, "text/plain", "Try to connect to wifi");

    pause();
    if (WiFi.getMode() == WIFI_MODE_AP) {
      wifi.setMode(WIFI_AP_STA);
    }

    log_i("request to connect to SSID: %s", ssid.c_str());
    wifi.setSSID(ssid);
    wifi.setPassword(password);
  });

  server.on("/wifi-state", HTTP_GET, [&](AsyncWebServerRequest* request) {
    return request->send(200, "text/plain", wifi.state().c_str());
  });

  server.on("/wifi-networks", HTTP_GET, [&](AsyncWebServerRequest* request) {
    return request->send(200, "text/plain", wifi.availableNetworks().c_str());
  });

  server.on("/wifi-start-scan", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (wifi.isScanning()) {
      return request->send(503, "text/plain", "Scanning");
    }
    wifi.requestScan();

    return request->send(200, "text/plain", "Scan started");
  });

  server.on("/wifi-forget", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("ssid")) {
      return request->send(400, "text/plain", "Error: no ssid parameter");
    }
    String ssid = request->getParam("ssid")->value();
    wifi.forgetNetwork(ssid);
    return request->send(200, "text/plain", "Network forgotten");
  });

  server.on("/logs", HTTP_GET, [&](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", logs.readLogs());
  });

  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Pong");
  });
  server.on("/conf-debug", HTTP_GET, [&](AsyncWebServerRequest* request) {
    // request->send(200, "text/plain", config.current_config.dump().c_str());
  });

  // batt is temporarily as a request since I don't want it to be polled as fast as the pipo data
  server.on("/battlevel", HTTP_GET, [&](AsyncWebServerRequest* request) {
    // Return battery percentage as integer, or -1 if plugged
    if (battery_plugged) {
      return request->send(200, "text/plain", "-1");
    } else {
      return request->send(200, "text/plain",
                           String(hwui.get_bat_percentage_int()).c_str());
    }
  });

  server.on("/offsetcal", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("axis")) {
      return request->send(400, "text/plain", "No sensor provided");
    }
    try {
      string axis = request->getParam("axis")->value().c_str();
      log_d("%s", axis.c_str());

      // Check if measurement is already in progress
      if (input_sensor.is_offset_measurement_complete()) {
        input_sensor.clear_completion_flag();
      }

      input_sensor.start_measure_offset(axis);

      // Return immediately with status - UI will poll for completion
      return request->send(202, "application/json",
                           "{\"status\":\"measuring\",\"axis\":\"" +
                               String(axis.c_str()) + "\"}");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error measuring offset: " + String(e.what()));
    }
  });

  // New endpoint to check offset measurement status
  server.on("/offsetcal-status", HTTP_GET, [&](AsyncWebServerRequest* request) {
    try {
      if (input_sensor.is_offset_measurement_complete()) {
        String offsetJson;
        input_sensor.get_measured_offsets(offsetJson);
        input_sensor.clear_completion_flag();

        return request->send(
            200, "application/json",
            "{\"status\":\"complete\",\"offsets\":" + offsetJson + "}");
      }
      return request->send(200, "application/json",
                           "{\"status\":\"measuring\"}");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error checking offset status: " + String(e.what()));
    }
  });

  server.on("/offsetcal-list", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("channels")) {
      return request->send(400, "text/plain", "No channel list provided");
    }
    try {
      string channels = request->getParam("channels")->value().c_str();
      log_i("Starting offset calibration for channels: %s", channels.c_str());

      // Check if measurement is already in progress
      if (input_sensor.is_offset_measurement_complete()) {
        input_sensor.clear_completion_flag();
      }

      input_sensor.start_measure_offset_list(channels);

      // Return immediately with status - UI will poll for completion
      return request->send(202, "application/json",
                           "{\"status\":\"measuring\"}");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error measuring offset: " + String(e.what()));
    }
  });

  server.on("/resetoffset", HTTP_POST, [&](AsyncWebServerRequest* request) {
    try {
      if (request->hasParam("axis")) {
        // Reset single axis
        string axis = request->getParam("axis")->value().c_str();
        input_sensor.reset_offset(axis);
        return request->send(200, "text/plain",
                             "Offset reset for " + String(axis.c_str()));
      } else {
        // Reset all offsets
        input_sensor.reset_all_offset();
        return request->send(200, "text/plain", "All offsets reset");
      }
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error resetting offset: " + String(e.what()));
    }
  });

#ifdef PIPO_MOTION
  server.on("/setreference", HTTP_GET, [&](AsyncWebServerRequest* request) {
    input_sensor.reset_reference_orientation();
    return request->send(200, "text/plain", "Reference orientation reset");
  });

  server.on("/relative-mode", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (input_sensor.get_relative_mode()) {
      return request->send(200, "text/plain", "true");
    } else {
      return request->send(200, "text/plain", "false");
    }
  });
#endif

#ifdef PIPO_RANGE
  server.on("/hold-mode", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (input_sensor.get_hold_mode()) {
      return request->send(200, "text/plain", "true");
    } else {
      return request->send(200, "text/plain", "false");
    }
  });
#endif

  // pause Engine
  server.on("/pause", HTTP_POST, [&](AsyncWebServerRequest* request) {
    PAUSED = true;
    return request->send(200, "text/plain", "Engine paused");
  });

  server.on("/resume", HTTP_POST, [&](AsyncWebServerRequest* request) {
    PAUSED = false;
    return request->send(200, "text/plain", "Engine resumed");
  });

  server.on("/is-paused", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (PAUSED) {
      return request->send(200, "text/plain", "true");
    } else {
      return request->send(200, "text/plain", "false");
    }
  });

  // Add preset routes
  presets.addRoutes(&server);

  // Solution by using Chunk Hanlder
  fileServer->setDefaultFile("index.html");
  server.addHandler(fileServer);
}

//TODO should use shared flag
bool pipoNetworkReady() {
  return wifi.ready() && server.isRunning();
}