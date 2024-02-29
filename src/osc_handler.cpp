#include "osc_handler.h"



/// @brief start the UDP connection. previouls
void OSC_handler::start() {
    if (dest_ip == IPAddress(0,0,0,0) || out_port == 0){
        Serial.println("Can't start OSC, No destination IP or port set");
        return;
    }
    else {
        Udp.begin(out_port);
        isStarted = true;
    }
}

void OSC_handler::stop() {
    Udp.stop();
    isStarted = false;
}

/// @brief use to update the destination IP
void OSC_handler::setDestIp(IPAddress ip) {
    if (!isStarted){
        dest_ip = ip;
    }
    if (isStarted){
        stop();
        dest_ip = ip;
        start();
    }
}

/// @brief use to update the output port 
void OSC_handler::setoutPort(int port) {
    if (!isStarted){
        out_port = port;
    }
    if (isStarted){
        stop();
        out_port = port;
        start();
    }
}

void OSC_handler::sendOscMessage(int value) {
    if (dest_ip != IPAddress(0,0,0,0) && out_port != 0){
        OSCMessage msg("/test");
        msg.add(value);
        Udp.beginPacket(dest_ip, out_port);
        msg.send(Udp);
        Udp.endPacket();
        msg.empty();
    }
    else {
        Serial.println("No destination IP or port set");
    }
}
