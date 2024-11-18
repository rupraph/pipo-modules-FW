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

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "DELETE, POST, GET, OPTIONS");
  DefaultHeaders::Instance().addHeader(
      "Access-Control-Allow-Headers",
      "Origin, X-Requested-With, Content-Type, Accept");
  server.onNotFound([](AsyncWebServerRequest* request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });
  server.serveStatic("/", LittleFS, "/webpage/").setDefaultFile("index.html");
  ws.enable(true);
  setup_requests();
  setup_ws();

  server.begin();
  Serial.println("Server setup donce");
  is_running = true;
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

void PipoServer::stop() {
  server.end();
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
      Serial.println(ESP.getFreeHeap());
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
            Serial.println(ESP.getFreeHeap());  // 44k remaining
#endif
            config.save(config.filename, received_configData.c_str());
            config.load_config(config.filename);
            config.apply(engine, osc, true);
#ifdef DEBUG_HEAP
            Serial.println(ESP.getFreeHeap());
#endif
#ifdef DEBUG_HEAP
            Serial.println(ESP.getFreeHeap());
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
    delay(3000);
    ESP.restart();
  });

  server.on("wifimode", HTTP_GET, [&](AsyncWebServerRequest* request) {
    if (config.general_config["Wifi_mode"] == "AP") {
      // Todo: should use setter
      config.general_config["Wifi_mode"].clear();
      config.general_config["Wifi_mode"] = "STA";
      return request->send(200, "text/plain", "switch to STA");
    } else {
      config.general_config["Wifi_mode"].clear();
      config.general_config["Wifi_mode"] = "switch to AP";
      return request->send(200, "text/plain", "STA");
    }
    // config.save(config.filename);
    // delay(1000);
    // ESP.restart();
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
    }
  } catch (const std::exception& e) {
    logs.writeError("error on message" + String(e.what()));
    Serial.println("error on message");
    Serial.println(e.what());
  }
}
void PipoServer::setup_ws() {
  server.addHandler(&ws);
  pipoSocket.setup(&ws);
  events.onConnect([](AsyncEventSourceClient* client) {});
  server.addHandler(&events);
  String msg = "";
  ws.onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client,
                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
      ws.cleanupClients(1);
    } else if (type == WS_EVT_ERROR) {
      ws.cleanupClients(1);
      Serial.print("ws error");
      Serial.print(server->url());
      Serial.print(client->id());
      Serial.print(*((uint16_t*)arg));
      Serial.println((char*)data);
    } else if (type == WS_EVT_PONG) {
      Serial.print("ws pong");
      Serial.print(server->url());
      Serial.print(client->id());
      Serial.print(len);
      Serial.println((len) ? (char*)data : "");
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len) {
        memcpy((void*)ws_message, data, info->len);
        ws_message_len = info->len;
        ws_message[ws_message_len] = 0;
        onMessage(client);
        return;
      }
      if (info->len >= ws_max_len - 1) {
        ws_message_len = 0;
        ws_message[0] = 0;
        return;
      }
      if (info->index == 0) {
        if (info->len + ws_message_len >= ws_max_len - 1) {
          ws_message_len = 0;
          ws_message[0] = 0;
          return;
        }
        memcpy((void*)(ws_message + ws_message_len), data, info->len);
        ws_message_len += info->len;
        if (info->index + len == info->len && info->final) {
          ws_message[ws_message_len] = 0;
          onMessage(client);
          ws_message_len = 0;
          ws_message[0] = 0;
        }
      }
    }
  });
}