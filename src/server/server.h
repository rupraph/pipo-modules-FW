#ifndef PIPOSERVER_H
#define PIPOSERVER_H
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "engine.h"
#include "utils/config.h"
#include "utils/json.hpp"
#include "utils/logs.h"
#include "utils/fs_tools.h"
#include "server/midi_socket.h"
#include "HW_CONFIG.h"


using namespace std;

class PipoServer
{
    public:
        PipoServer(Sensor& sensor, Engine& engine, OSC_handler& osc): server(80), ws("/ws"),
        events("/events"), input_sens(sensor),engine(engine),osc(osc){}
        void setup();
        void setup_ws();
        void onMessage(AsyncWebSocketClient* client, String message);
        void setup_requests();
        void stop(); // Todo: start stop not much tested yet
        bool is_running = false;

    private:
        AsyncWebServer server;
        AsyncWebSocket ws;
        AsyncEventSource events;
        Sensor& input_sens;
        Engine& engine;
        OSC_handler& osc;
        string received_configData;
        json parsed_configData;

};
#endif  // WEBSERVER_H