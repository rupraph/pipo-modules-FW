#include "server/server.h"

PipoServer server;

void PipoServer::setup() {
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "DELETE, POST, GET, OPTIONS");
  DefaultHeaders::Instance().addHeader(
      "Access-Control-Allow-Headers",
      "Origin, X-Requested-With, Content-Type, Accept");
  std::string mdns_name = std::string("pipo-") + PIPO_TYPE;
  if (!MDNS.begin(
          mdns_name.c_str())) {  // Start the mDNS responder for esp.local
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
  Serial.println("Start server");
  //Todo: check lib exemple. can be improved
  server.onNotFound([&](AsyncWebServerRequest* request) {
    Serial.println("not found: " + request->url());
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });
  server.serveStatic("/", LittleFS, "/webpage/").setDefaultFile("index.html");
  setup_requests();
  // captivePortal.start(&server);
  pipoSocket.start(&ws);
  server.addHandler(&ws);
  pipoSocket.setup();
  server.begin();
  is_running = true;

#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif
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
    Serial.println("info request");
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
      config.set(request->getParam("config")->value());
      pipoDebugHeap();
      config.apply(engine, osc, DEBUG_CONFIG);
      config.save();
      return request->send(200, "text/plain", "Config set");
    } catch (std::exception e) {
      return request->send(500, "text/plain",
                           "Error while setting config: " + String(e.what()));
    }
  });

  // sends config-list and config file based on provided filename
  server.on("/configs", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("name")) {
      return request->send(200, "text/plain", config.get_list());
    }
    try {
      String name = request->getParam("name")->value();
      pipoDebugHeap();
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
      config.load_config(request->getParam("name")->value().c_str(), true);
      config.apply(engine, osc, DEBUG_CONFIG);
      return request->send(200, "text/plain", "Active config set");
    } catch (const std::exception e) {
      Serial.println("error loading config");
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
          }
          received_configData.append((char*)data, len);

          if (final) {
        // This is the end of the file upload
        // Here I am doing save first then load. so parsing happen with load function.
        // this avoids parsing in here and trying to pass the json to config.set().
        // after solving other issues, not sure if this has any value after all.

#ifdef DEBUG_HEAP
            pipoDebugHeap();
#endif
            config.save(config.filename, received_configData.c_str());
            config.load_config(config.filename);
            config.apply(engine, osc, DEBUG_CONFIG);
#ifdef DEBUG_HEAP
            pipoDebugHeap();
#endif
#ifdef DEBUG_HEAP
            pipoDebugHeap();
#endif
            received_configData.clear();
            return request->send(200, "text/plain", "Config saved");
          }
        } catch (const std::exception& e) {
          Serial.println("error saving config");
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
    Serial.println("POST wifi-mode");
    if (!request->hasParam("mode")) {
      Serial.println("no mode");

      return request->send(400, "text/plain", "Error: no mode parameter");
    }
    String mode = request->getParam("mode")->value();
    Serial.println("mode: " + mode);
    if (mode != "AP" && mode != "STA" && mode != "APSTA") {
      return request->send(400, "text/plain", "Error: invalid mode");
    }
    request->send(200, "text/plain", "Try to switch to mode " + mode);
    vTaskDelay(pdMS_TO_TICKS(100));
    pause();
    config.general_config["Wifi_mode"] = mode;
    Serial.println("Setting mode: " + mode);
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
      return request->send(400, "text/plain", "Error: no ssid  parameter");
    }
    request->send(200, "text/plain", "Try to connect to wifi");
    String ssid = request->getParam("ssid")->value();
    String password = "";
    if (request->hasParam("password")) {
      password = request->getParam("password")->value();
    }
    String previous_ssid = wifi.ssid();
    bool isAPSTA = WiFi.getMode() == WIFI_MODE_APSTA;
    pause();
    if (WiFi.getMode() == WIFI_MODE_AP) {
      wifi.setMode(WIFI_AP_STA);
    }
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
    return request->send(200, "text/plain",
                         String(hwui.get_bat_voltage()).c_str());
  });

  // Todo: this is too long to be executed in the server reauest
  // this should be offloaded to a task and a monitoring task setup to  handle and send the pending response when the action if finished
  server.on("/offsetcal", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("axis")) {
      return request->send(400, "text/plain", "No sensor provided");
    }
    try {
      string axis = request->getParam("axis")->value().c_str();
      Serial.println(axis.c_str());
      input_sensor.measure_offset(axis);
      config.gather(engine);
      config.save();
      return request->send(200, "text/plain", "Offset measured");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error measuring offset: " + String(e.what()));
    }
  });
#ifdef PIPO_ANALOG
  server.on("/offsetAllTouch", HTTP_POST, [&](AsyncWebServerRequest* request) {
    try {
      input_sensor.measure_offset_all();
      config.gather(engine);
      config.save();
      return request->send(200, "text/plain", "Offset measured");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error measuring offset: " + String(e.what()));
    }
  });
#endif

  server.on("/pause", HTTP_GET, [&](AsyncWebServerRequest* request) {
    engine.toggle_pause();
    return request->send(200, "text/plain", "Engine paused");
  });
}

bool pipoNetworkReady() {
  return wifi.ready() && server.isRunning();
}