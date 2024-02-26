#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H
#include <OSCMessage.h>
#include <WiFiUDP.h>

WiFiUDP Udp;

class OSC_handler {
    public:
        IPAddress dest_ip;
        int out_port;
        void setup(int port);
        void sendOscMessage(OSCMessage msg);
};

#endif //OSC_HANDLER_H