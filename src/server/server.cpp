#include "server/server.h"

PipoServer server;

void PipoServer::setup() {
  //Todo: check lib exemple. can be improved
  std::string mdns_name = std::string("pipo-") + PIPO_TYPE;
  if (!MDNS.begin(
          mdns_name.c_str())) {  // Start the mDNS responder for esp.local
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
  start();
#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif
}
void PipoServer::start() {
  Serial.println("Start server");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "DELETE, POST, GET, OPTIONS");
  DefaultHeaders::Instance().addHeader(
      "Access-Control-Allow-Headers",
      "Origin, X-Requested-With, Content-Type, Accept");
  server.onNotFound([&](AsyncWebServerRequest* request) {
    Serial.println("not found: " + request->url());
    if (captivePortal.is_active()) {
      auto url = "http://" + WiFi.softAPIP().toString();
      // return request->redirect(url);
      return request->redirect("/");
    }
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });
  server.serveStatic("/", LittleFS, "/webpage/").setDefaultFile("index.html");
  ws.enable(true);
  setup_requests();
  captivePortal.start(&server);
  setup_ws();
  server.begin();
  is_running = true;
  should_start = false;
}
void PipoServer::stop() {
  pipoSocket.stop();
  server.end();
  server.reset();
  ws_initialized = false;
  DefaultHeaders::Instance().clear();
  captivePortal.stop();
  vTaskDelay(pdMS_TO_TICKS(100));
  Serial.println("end server");
  is_running = false;
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

  // recevies and apply config
  server.on("/config", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("config")) {
      return request->send(400, "text/plain", "No config received");
    }
    try {
      config.set(request->getParam("config")->value());
      pipoDebugHeap();
      config.apply(engine, osc, true);
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
      config.apply(engine, osc, true);
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
      config.apply(engine, osc, true);
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
            config.apply(engine, osc, true);
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
    stop();
    config.general_config["Wifi_mode"] = mode;
    Serial.println("Setting mode: " + mode);
    if (mode == "AP") {
      wifi.APMode();
    } else if (mode == "STA") {
      wifi.STAMode();
    } else {
      wifi.APSTAMode();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    should_start = true;
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
    vTaskDelay(pdMS_TO_TICKS(200));
    stop();
    bool success = false;
    success =
        password.length() ? wifi.connect(ssid, password) : wifi.connect(ssid);
    Serial.println("Connected ? ");
    if (!success && previous_ssid.length()) {
      vTaskDelay(pdMS_TO_TICKS(200));
      Serial.println("Not Connected!, reconnect to previous");
      wifi.connect(previous_ssid);
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    should_start = true;
  });

  server.on("/wifi-state", HTTP_GET, [&](AsyncWebServerRequest* request) {
    return request->send(200, "text/plain", wifi.state().c_str());
  });

  server.on("/wifi-networks", HTTP_GET, [&](AsyncWebServerRequest* request) {
    return request->send(200, "text/plain", wifi.availableNetworks().c_str());
  });

  server.on("/wifi-scan", HTTP_GET, [&](AsyncWebServerRequest* request) {
    wifi.scan();
    return request->send(200, "text/plain", "wifi scan done");
  });

  server.on("/logs", HTTP_GET, [&](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", logs.readLogs().c_str());
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

  server.on("/offsetcal", HTTP_POST, [&](AsyncWebServerRequest* request) {
    if (!request->hasParam("axis")) {
      return request->send(400, "text/plain", "No sensor provided");
    }
    try {
      string axis = request->getParam("axis")->value().c_str();
      Serial.println(axis.c_str());
      input_sensor.measure_offset(axis);
      return request->send(200, "text/plain", "Offset measured");
    } catch (const std::exception& e) {
      return request->send(500, "text/plain",
                           "Error measuring offset: " + String(e.what()));
    }
  });

  server.on("/pause", HTTP_GET, [&](AsyncWebServerRequest* request) {
    engine.toggle_pause();
    return request->send(200, "text/plain", "Engine paused");
  });
}

void PipoServer::onMessage(AsyncWebSocketClient* client) {
  try {
    char command[16];
    int offset = 0;
    int i = 0;
    for (i = 0; i < ws_message_len; i++) {
      if (ws_message[i] == ':') {
        command[offset] = 0;
        break;
      }
      command[offset++] = ws_message[i];
    }

    if (strcmp("config", command) == 0) {
      config.setValue(ws_message + offset + 1, ws_message_len - offset - 1);
      config.apply(engine, osc, true);
    } else if (strcmp("configs", command) == 0) {
      config.setValues(ws_message + offset + 1, ws_message_len - offset - 1);
      config.apply(engine, osc, true);
    } else if (strcmp("save", command) == 0) {
      config.save();
    } else if (strcmp("monitor", command) == 0) {
      input_sensor.monitor_axis(ws_message + offset + 1);
    }
  } catch (const std::exception& e) {
    logs.writeError("error on message" + String(e.what()));
    Serial.println("error on message");
    Serial.println(e.what());
  }
}

void PipoServer::setup_ws() {
  if (ws_initialized) {
    pipoSocket.setup(&ws);
    return;
  }
  ws_initialized = true;
  server.addHandler(&ws);
  pipoSocket.setup(&ws);
  events.onConnect([](AsyncEventSourceClient* client) {});
  server.addHandler(&events);
  ws.onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client,
                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("Client connected: ID = %u, URL = %s\n", client->id(),
                    server->url());
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("Client disconnected: ID = %u, URL = %s\n", client->id(),
                    server->url());
    } else if (type == WS_EVT_ERROR) {
      uint16_t errorCode = *((uint16_t*)arg);
      Serial.printf(
          "WebSocket error: URL = %s, Client ID = %u, Error Code = %d, Data = "
          "%s\n",
          server->url(), client->id(), errorCode, (char*)data);
    } else if (type == WS_EVT_PONG) {
      Serial.printf("Pong received: URL = %s, Client ID = %u, Data = %s\n",
                    server->url(), client->id(),
                    (len) ? (char*)data : "No Data");
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;

      if (info->index == 0 && !info->final) {
        // Start of a fragmented message
        ws_message_len = 0;
      }

      if (info->index + len <= ws_max_len - 1) {
        // Append the current fragment to the message buffer
        memcpy(ws_message + info->index, data, len);
        ws_message_len = info->index + len;

        if (info->final) {
          // Message is complete
          ws_message[ws_message_len] = '\0';  // Null-terminate the message
          onMessage(client);                  // Process the complete message
          ws_message_len = 0;                 // Reset for the next message
        }
      } else {
        // Message too large or buffer overflow
        Serial.println("Error: Message exceeds buffer size");
        ws_message_len = 0;
      }
    }
  });
}