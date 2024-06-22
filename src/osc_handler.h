#ifndef OSC_HANDLER_H
#define OSC_HANDLER_H
#include <OSCMessage.h>
#include <WiFiUdp.h>

class OSC_handler
{
public:
    WiFiUDP Udp;
    IPAddress dest_ip;
    int out_port;
    bool isStarted = false;
    void start();
    void stop();
    void sendOscMessage(int value);
    void setDestIp(IPAddress ip);
    void setoutPort(int port);
};

#endif // OSC_HANDLER_H