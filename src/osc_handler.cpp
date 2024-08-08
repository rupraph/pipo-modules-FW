#include "osc_handler.h"



/// @brief setup the OSC handler
void OSC_handler::setup() {
    //dest_ip = config.general_config["OSC_IP"];
    out_port = config.general_config["OSC_PORT"];
    //Serial.println("OSC IP: " + dest_ip.toString());
    Serial.println("OSC PORT: " + String(out_port));
    if (config.general_config["OSC_ENA"]){
        //start();
    }
}

/// @brief start the UDP connection. 
void OSC_handler::start() {
    if (dest_ip == IPAddress(0,0,0,0) || out_port == 0){
        Serial.println("Can't start OSC, No destination IP or port set");
        return;
    }
    else {
        Udp.begin(out_port);
        isStarted = true;
        Serial.println("OSC started");
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

void OSC_handler::sendOscMessage(const char* address,int value) {
    if (dest_ip != IPAddress(0,0,0,0) && out_port != 0){
        OSCMessage msg(address);
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
