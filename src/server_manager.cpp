#include "server_manager.h"
#include "utils/fs_tools.h"
//#include <ArduinoJson.h>


// is using buildflag for regex
// ASYNCWEBSERVER_REGEX to enable the regex support
// For platformio: platformio.ini:
//  build_flags = 
//      -DASYNCWEBSERVER_REGEX

using json = nlohmann::json;

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

    // server.on("/config/save", HTTP_POST, [this](AsyncWebServerRequest *request){
    //     request->send(200, "text/plain", "Hello, world");
    //     this->config.gather_current_config(this->acc_sensor,this->engine,true);
    //     this->config.save_config("/config/current_config.json");
    // });

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

    
    
    ////////// CONFIG FORM 
    // attemp to create a draft config page
    server.on("/configdraft", HTTP_GET, [this](AsyncWebServerRequest *request){
        json configJson = this->engine.get_config();  // Get the current config
        try
        {
            String html = String(generateHtmlForm(configJson).c_str());  // Generate the HTML form
            request->send(200, "text/html", html);  // Send the HTML form
        }
        catch(const std::exception& e)
        {
            Serial.println("error");
            Serial.println(e.what());
        }
        
        // Send the HTML form
    });

    // aplly  config
    server.on("/config/save", HTTP_POST, [this](AsyncWebServerRequest *request){
        json newConfig;
        for (int i=0; i<request->params(); i++) {
            AsyncWebParameter* p = request->getParam(i);
            newConfig[p->name().c_str()] = p->value().c_str();
        }

        // Update the engine config
        this->config.set_current_config(newConfig);
        this->config.apply_current_config(this->input_sens,this->engine,true);//this->acc_sensor,

        // Save the new config to disk
        //this->config.save_config_to_file("/config/current_config.json", newConfig);

        request->send(200, "text/plain", "Config updated successfully");
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


// this is missing sensor settings
void ServerManager::generateHtmlForm(json& configJson, string& html, string prefix) {
    for (json::iterator it=configJson.begin(); it!=configJson.end(); ++it) {
        string key = prefix + it.key();
        if (it.value().is_object()) {
            // If the value is a JSON object, recurse into it
            generateHtmlForm(it.value(), html, key + ".");
        } else {
            // Otherwise, generate an input field for the value
            html += "<label for=\"" + key + "\">" + key + ":</label><br>";
            if (it.value().is_string()) {
                html += "<input type=\"text\" id=\"" + key + "\" name=\"" + key + "\" value=\"" + it.value().get<string>() + "\"><br>";
            } else if (it.value().is_number()) {
                html += "<input type=\"number\" id=\"" + key + "\" name=\"" + key + "\" value=\"" + to_string(it.value().get<int>()) + "\"><br>";
            } else if (it.value().is_boolean()) {
                html += "<input type=\"checkbox\" id=\"" + key + "\" name=\"" + key + "\" " + (it.value().get<bool>() ? "checked" : "") + "><br>";
            }
        }
    }
}

string ServerManager::generateHtmlForm(json& configJson) {
    string html = "<html><body><form action=\"/config/save\" method=\"post\">";
    generateHtmlForm(configJson, html);
    html += "<input type=\"submit\" value=\"Submit\">";
    html += "</form></body></html>";
    return html;
}
