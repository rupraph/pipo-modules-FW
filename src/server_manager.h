#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "engine.h"


class ServerManager
{
    public:
        ServerManager(Engine& engine): server(80), engine(engine) {}
        void setup();
        void setup_requests();
        static void notFound(AsyncWebServerRequest *request);

    private:
        AsyncWebServer server;
        sensor& acc_sensor = sensor::getInstance();
        Engine& engine;
};

#endif //WEBSERVER_H