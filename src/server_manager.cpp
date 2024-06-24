#include "server_manager.h"

#include "utils/fs_tools.h"
//#include <ArduinoJson.h>

// is using buildflag for regex
// ASYNCWEBSERVER_REGEX to enable the regex support
// For platformio: platformio.ini:
//  build_flags =
//      -DASYNCWEBSERVER_REGEX

using json = nlohmann::json;

void ServerManager::setup() {
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

    setup_requests();

    server.begin();
    is_running = true;
}

void ServerManager::stop() {
    server.end();
    is_running = false;
}

void ServerManager::setup_requests() {
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