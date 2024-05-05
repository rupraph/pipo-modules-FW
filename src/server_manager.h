#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "engine.h"
#include "config.h"

class ServerManager
{
    public:
        ServerManager(Engine& engine,Config& config): server(80), engine(engine),config(config) {}
        void setup();
        void setup_requests();
        static void notFound(AsyncWebServerRequest *request);

    private:
        AsyncWebServer server;
        sensor& acc_sensor = sensor::getInstance();
        Engine& engine;
        Config& config;

};

#endif //WEBSERVER_H