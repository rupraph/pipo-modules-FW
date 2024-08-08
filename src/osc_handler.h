#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#include <OSCMessage.h>
#include <WiFiUdp.h>
#include "utils/config.h"

class Config;

class OSC_handler
{
    public:
        OSC_handler(Config& config): config(config) {}
        
        void setup();
        void start();
        void stop();
        void sendOscMessage(const char* address, int value);
        void setDestIp(IPAddress ip);
        void setoutPort(int port);
        
    private:
        Config& config;
        IPAddress dest_ip;
        int out_port;
        bool isStarted = false;
        WiFiUDP Udp;

};

#endif // OSC_HANDLER_H