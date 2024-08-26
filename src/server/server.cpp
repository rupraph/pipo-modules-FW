#include "server/server.h"

using json = nlohmann::json;


void PipoServer::setup() {
    // not sure this is the best way to do this. see exemples
    
    // name should depend on the type of pipo, but for now config page has no mean to retrive the right type to connect to it. 
    std::string mdns_name=std::string("pipo-")+PIPO_TYPE;
    if (!MDNS.begin(mdns_name.c_str())) {  // Start the mDNS responder for esp.local
        Serial.println("Error setting up MDNS responder!");
    } else {
        Serial.println("mDNS responder started");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 80);
    }

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "DELETE, POST, GET, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
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
        String type;
        json info = {
            {"name", "unnamed Pipo"},// should come from config file
            {"version", string(PIPO_FW_VERSION)}, // should come from HW_CONFIG
            {"type", string(PIPO_TYPE)},
            {"ip", WiFi.localIP().toString().c_str()},
            {"mac", WiFi.macAddress().c_str()},
        };
        return request->send(200, "text/json", info.dump().c_str());
    });

    // server.on("/config", HTTP_GET,[&](AsyncWebServerRequest* request){
    //     return request->send(200, "text/plain", config.get().dump().c_str()); 
    // });

    
    server.on("/config", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (!request->hasParam("config")) {
            return request->send(400, "text/plain", "No config received");
        }
        try {
            config.set(json::parse(request->getParam("config")->value()));
            config.apply(input_sens, engine, osc, true);
            return request->send(200, "text/plain", "Config set");
        } catch (std::exception e) {
            return request->send(500, "text/plain", "Error while setting config: " + String(e.what()));
        }
    });

        // sends config with filename
    server.on("/configs", HTTP_GET, [&](AsyncWebServerRequest* request) {
        if(!request->hasParam("name")){
            return request->send(200, "text/plain", config.get_list());
        }
        try{
            String name = request->getParam("name")->value(); 
            Serial.println(ESP.getFreeHeap());
            return request->send(LittleFS, config.get_path(name), "application/json");
        }
        catch(const std::exception e){
            return request->send(500, "text/plain", "Error loading config: " + String(e.what()));
        }
    });

    server.on("/config-active", HTTP_GET,[&](AsyncWebServerRequest* request) { 
        return request->send(200, "text/plain", config.filename.c_str()); 
        });

    server.on("/active-config", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (!request->hasParam("name")) {
            return request->send(400, "text/plain", "Error: no name parameter");
        }
        try {
            config.load_config(request->getParam("name")->value().c_str(), true);
            config.apply(input_sens, engine, osc, true);
            return request->send(200, "text/plain", "Active config set");
        } catch (const std::exception e) {
            Serial.println("error loading config");
            return request->send(500, "text/plain", "Error loading config: " + String(e.what()));
        }
    });
     server.on("/config-delete", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (!request->hasParam("name")) {
            return request->send(400, "text/plain", "Error: no name parameter");
        }
        try {
            config.delete_config(request->getParam("name")->value());
            config.apply(input_sens, engine,osc, true);
            return request->send(200, "text/plain", "Config deleted");
        } catch (const std::exception e) {
            return request->send(500, "text/plain", "Error deleting config: " + String(e.what()));
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
            return request->send(500, "text/plain", "Error creating config: " + String(e.what()));
        }
    });
    server.on("/config-copy", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (!request->hasParam("name") || !request->hasParam("config")) {
            return request->send(400, "text/plain", "Error: no name or config parameter");
        }
        try {
            config.save(request->getParam("name")->value(), request->getParam("config")->value());
            return request->send(200, "text/plain", "Config copied");
        } catch (const std::exception e) {
            return request->send(500, "text/plain", "Error copying config: " + String(e.what()));
        }
    });
     server.on("/config-rename", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (!request->hasParam("oldname") || !request->hasParam("newname")) {
            return request->send(400, "text/plain", "Error: no old or new name parameter");
        }
        try {
            config.rename(request->getParam("oldname")->value(), request->getParam("newname")->value());
            return request->send(200, "text/plain", "Config renamed");
        } catch (const std::exception e) {
            return request->send(500, "text/plain", "Error renaming config: " + String(e.what()));
        }
    });


    server.on("/save", HTTP_POST, 
        [&](AsyncWebServerRequest* request) {
        return request->send(200, "text/plain", "Config sending");
        },
        [&](AsyncWebServerRequest* request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
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
                    
                    config.save(config.filename+".json", received_configData.c_str());
                    
                    #ifdef DEBUG_HEAP
                            Serial.println(ESP.getFreeHeap());
                    #endif
                    
                    config.load_config(config.filename, true);
                    
                    #ifdef DEBUG_HEAP
                            Serial.println(ESP.getFreeHeap());
                    #endif
                    
                    received_configData.clear();  
                    config.apply(input_sens, engine, osc, true);
                    return request->send(200, "text/plain", "Config saved");
                }
            }catch (const std::exception& e) {
                Serial.println("error saving config");
                return request->send(500, "text/plain", "Error saving config: " + String(e.what()));
            }
        }
    );

    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest* request) {
        return request->send(200, "text/plain", "Rebooting");
        delay(1000);
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
        config.save(config.filename);
        delay(1000);
        ESP.restart();
    });

    server.on("/logs", HTTP_GET,[&](AsyncWebServerRequest* request) { 
        request->send(200, "text/plain", logs.readLogs().c_str()); });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest* request) { 
        request->send(200, "text/plain", "Pong"); });
}
void PipoServer::onMessage(AsyncWebSocketClient* client, String message) {
    Serial.println(message);
    // client->text("I got your message");
}
void PipoServer::setup_ws() {
    server.addHandler(&ws);
    events.onConnect([](AsyncEventSourceClient* client) { client->send("hello!", NULL, millis(), 1000); });
    server.addHandler(&events);
    midisocket.setup(&ws);
    ws.onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data,
                   size_t len) {
        if (type == WS_EVT_CONNECT) {
            //Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
            Serial.print("ws connect");
            Serial.print(server->url());
            Serial.print(client->id());
            Serial.println();
            //client->printf("Hello Client %u :)", client->id());
            client->ping();
        } else if (type == WS_EVT_DISCONNECT) {
            //Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
            Serial.print("ws disconnect");
            Serial.print(server->url());
            Serial.print(client->id());
        } else if (type == WS_EVT_ERROR) {
            //Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
            Serial.print("ws error");
            Serial.print(server->url());
            Serial.print(client->id());
            Serial.print(*((uint16_t*)arg));
            Serial.println((char*)data);
        } else if (type == WS_EVT_PONG) {
            //Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
            Serial.print("ws pong");
            Serial.print(server->url());
            Serial.print(client->id());
            Serial.print(len);
            Serial.println((len) ? (char*)data : "");
        } else if (type == WS_EVT_DATA) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            String msg = "";
            if (info->final && info->index == 0 && info->len == len) {
                // the whole message is in a single frame and we got all of it's data
                if (info->opcode == WS_TEXT) {
                    for (size_t i = 0; i < info->len; i++) {
                        msg += (char)data[i];
                    }
                } else {
                    char buff[3];
                    for (size_t i = 0; i < info->len; i++) {
                        // sprintf(buff, "%02x ", (uint8_t)data[i]);
                        // msg += buff;
                        // removing sprintf to reduce memory usage
                        if (data[i] < 16) msg += '0'; // Add leading zero for single hex digit
                         msg += String((uint8_t)data[i], HEX);
                        msg += ' ';
                    }
                }
                //Serial.printf("%s\n", msg.c_str());
                Serial.print(msg);

                if (info->opcode == WS_TEXT) onMessage(client, msg);
            } else {
                // message is sent as multiple frames or the frame is split into multiple packets
                if (info->opcode == WS_TEXT) {
                    for (size_t i = 0; i < len; i++) {
                        msg += (char)data[i];
                    }
                } else {
                    char buff[3];
                    for (size_t i = 0; i < len; i++) {
                        // sprintf(buff, "%02x ", (uint8_t)data[i]);
                        // msg += buff;
                        // removing sprintf to reduce memory usage
                        if (data[i] < 16) msg += '0'; // Add leading zero for single hex digit
                        msg += String((uint8_t)data[i], HEX);
                        msg += ' ';
                    }
                }
                if ((info->index + len) < info->len) return;
                if (!info->final) return;
                if (info->message_opcode == WS_TEXT) onMessage(client, msg);
            }
        }
    });
}