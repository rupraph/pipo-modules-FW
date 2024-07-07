#include "server/server.h"

using json = nlohmann::json;

void PipoServer::setup() {
    // not sure this is the best way to do this. see exemples
    if (!MDNS.begin("Pipo-Motion")) {  // Start the mDNS responder for esp.local
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
    is_running = true;
}

void PipoServer::stop() {
    server.end();
    is_running = false;
}

void PipoServer::setup_requests() {
    server.on("/info", HTTP_GET, [&](AsyncWebServerRequest* request) {
        String type;

#if defined(PIPO_MOTION)
        type = "PIPO_MOTION";
#elif defined(PIPO_RANGE)
     type = "PIPO_RANGE";
#elif defined(PIPO_ANALOG)
    type = "PIPO_ANALOG";
#endif
        json info = {
            {"name", "unnamed Pipo"},
            {"version", "0.1"},
            {"type", type.c_str()},
            {"ip", WiFi.localIP().toString().c_str()},
            {"mac", WiFi.macAddress().c_str()},
        };
        request->send(200, "text/json", info.dump().c_str());
    });

    server.on("/config", HTTP_GET,
              [&](AsyncWebServerRequest* request) { request->send(200, "text/plain", config.get().dump().c_str()); });
    server.on("/config", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (request->hasParam("config")) {
            Serial.print("Received config ");
            Serial.println(request->getParam("config")->value());
            config.set(json::parse(request->getParam("config")->value()));
            config.apply(input_sens, engine, true);
        } else {
            Serial.print("No config received");
        }
        request->send(200, "text/plain", "Config set");
    });

    server.on("/config-save", HTTP_POST, [&](AsyncWebServerRequest* request) {
        if (request->hasParam("config")) {
            Serial.print("Received config ");
            Serial.println(request->getParam("config")->value());
            config.set(json::parse(request->getParam("config")->value()));
            config.apply(input_sens, engine, true);
            config.save();
        } else {
            Serial.print("No config received");
        }
        request->send(200, "text/plain", "Config set");
    });

    server.on("/logs", HTTP_GET,
              [&](AsyncWebServerRequest* request) { request->send(200, "text/plain", logs.readLogs().c_str()); });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest* request) { request->send(200, "text/plain", "Pong"); });
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
            Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
            client->printf("Hello Client %u :)", client->id());
            client->ping();
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
        } else if (type == WS_EVT_ERROR) {
            Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
        } else if (type == WS_EVT_PONG) {
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
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
                        sprintf(buff, "%02x ", (uint8_t)data[i]);
                        msg += buff;
                    }
                }
                Serial.printf("%s\n", msg.c_str());

                if (info->opcode == WS_TEXT)
                    onMessage(client, msg);
            } else {
                // message is sent as multiple frames or the frame is split into multiple packets
                if (info->opcode == WS_TEXT) {
                    for (size_t i = 0; i < len; i++) {
                        msg += (char)data[i];
                    }
                } else {
                    char buff[3];
                    for (size_t i = 0; i < len; i++) {
                        sprintf(buff, "%02x ", (uint8_t)data[i]);
                        msg += buff;
                    }
                }
                if ((info->index + len) < info->len) return;
                if (!info->final) return;
                if (info->message_opcode == WS_TEXT)
                    onMessage(client, msg);
            }
        }
    });
}