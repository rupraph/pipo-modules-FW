#include "osc_handler.h"


void OSC_handler::setup(int outport) {
    out_port = outport;
    Udp.begin(out_port);
}

void OSC_handler::sendOscMessage(OSCMessage msg) {
    Udp.beginPacket(dest_ip, out_port);
    msg.send(Udp);
    Udp.endPacket();
}
