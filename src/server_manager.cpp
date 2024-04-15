#include "server_manager.h"
#include "fs_tools.h"


AsyncWebServer server(80);

void webserver_setup(){

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
    server.onNotFound(notFound);

    setup_requests();

    server.begin();
    delay(1000);
}

void setup_requests(){
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

