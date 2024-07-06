#ifndef PIPOSERVER_H
#define PIPOSERVER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include "engine.h"
#include "utils/config.h"
#include "utils/json.hpp"
#include "utils/logs.h"
#include "utils/midi_logs.h"
using namespace std;

class PipoServer
{
    public:
        PipoServer(Sensor& sensor, Engine& engine): server(80), input_sens(sensor),engine(engine){}
        void setup();
        void setup_requests();
        void stop(); // Todo: start stop not much tested yet
        bool is_running = false;

    private:
        AsyncWebServer server;
        Sensor& input_sens;
        Engine& engine;

};
#endif  // WEBSERVER_H