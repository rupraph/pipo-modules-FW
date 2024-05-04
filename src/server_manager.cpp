#include "server_manager.h"
#include "fs_tools.h"




void ServerManager::setup(){

    // not sure this is the best way to do this. see exemples
    if (!MDNS.begin("Pipo-Motion")) { // Start the mDNS responder for esp.local
        Serial.println("Error setting up MDNS responder!");
    } else {
        Serial.println("mDNS responder started");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 80);
    }


    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    server.serveStatic("/", LittleFS, "/webpage/").setDefaultFile("index.html");
    
    server.onNotFound(notFound);

    setup_requests();

    server.begin();

}

void ServerManager::setup_requests(){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    server.on("/config/general", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    server.on("/config/midi", HTTP_POST, [this](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
        this->engine.Miditranslators["roll"].set_param("translator_mode", 1);
    });

    server.on("/config/osc", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    server.on("/config/hid", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });
}

void ServerManager::notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

