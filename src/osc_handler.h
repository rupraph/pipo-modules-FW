#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H

#include <OSCMessage.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "utils/config.h"

class Config;

class OSC_handler
{
    public:
        OSC_handler(Config& config): config(config) {}
        
        void setup();
        void set_config();
        void sendOscMessage(const char* address, float value);
        
        
    private:
        Config& config;
        IPAddress dest_ip;
        int out_port;
        bool isStarted = false;
        bool enabled = false;
        WiFiUDP Udp;
        void start();
        void stop();
        void setDestIp(string ip);
        void setOutPort(int port);
        void setEnabled(bool ena);

};

#endif // OSC_HANDLER_H