#include "server_manager.h"
#include "fs_tools.h"


// is using buildflag for regex
// ASYNCWEBSERVER_REGEX to enable the regex support
// For platformio: platformio.ini:
//  build_flags = 
//      -DASYNCWEBSERVER_REGEX


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

    server.on("^\\/config/midi\\/([a-zA-Z0-9]+)\\/([0-9]+)$", HTTP_GET, [this](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Midi"+request->pathArg(0)+"value"+request->pathArg(1));
    //this->engine.Miditranslators["roll"].set_param("translator_mode", 1);
    });

    server.on("/config/osc", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    server.on("/config/hid", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });


    /// download Config file interface

    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><ul>";
    File root = LittleFS.open("/config");
    File file = root.openNextFile();
    while(file){
        html += "<li><a href=\"/download?file=";
        html += file.name();
        html += "\">";
        html += file.name();
        html += "</a> - Last modified: ";
        // Get the last write time and format it as a string:
        time_t t = file.getLastWrite();
        struct tm *tmstruct = localtime(&t);
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tmstruct);
        html += timeStr;
        html += "</li>";
        file = root.openNextFile();
    }
    html += "</ul></body></html>";
    request->send(200, "text/html", html);
    });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("file")) {
        String filename = "/config/"+request->getParam("file")->value();
        Serial.println("Download request: " + filename);
        request->send(LittleFS, filename, "application/octet-stream",true);
    } else {
        request->send(400, "text/plain", "Bad request");
    }
});
}

void ServerManager::notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

