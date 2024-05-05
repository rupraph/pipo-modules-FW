#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "engine.h"
#include "config.h"
#include "json.hpp"

using namespace std;

class ServerManager
{
    public:
        ServerManager(Engine& engine,Config& config): server(80), engine(engine),config(config) {}
        void setup();
        void setup_requests();
        static void notFound(AsyncWebServerRequest *request);
        void generateHtmlForm(json& configJson,string& html,string prefix="");
        string generateHtmlForm(json& configJson);

    private:
        AsyncWebServer server;
        sensor& acc_sensor = sensor::getInstance();
        Engine& engine;
        Config& config;

};

#endif //WEBSERVER_H