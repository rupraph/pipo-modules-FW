#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "engine.h"
#include "utils/config.h"
#include "utils/json.hpp"

using namespace std;

class ServerManager
{
    public:
        ServerManager(Sensor& sensor, Engine& engine,Config& config): server(80), input_sens(sensor),engine(engine),config(config) {}
        void setup();
        void setup_requests();
        static void notFound(AsyncWebServerRequest *request);
        void generateHtmlForm(json& configJson,string& html,string prefix="");
        string generateHtmlForm(json& configJson);

    private:
        AsyncWebServer server;
        Sensor& input_sens;
        Engine& engine;
        Config& config;

};

#endif //WEBSERVER_H