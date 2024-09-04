#include <server/pipo_socket.h>

PipoSocket pipoSocket;
PipoSocket::PipoSocket() { 
    this->ws = nullptr;
}
void PipoSocket::setup(AsyncWebSocket* ws) { this->ws = ws; }
void PipoSocket::sendNoteOn(int note, int velocity, int channel) {
    if (ws == nullptr) return;
    String msg = "noteon";
    msg += channel;
    msg += ",";
    msg += note;
    msg += ",";
    msg += velocity;
    ws->textAll(msg.c_str());
}
void PipoSocket::sendNoteOff(int note, int velocity, int channel) {
    if (ws == nullptr) return;
    String msg = "noteoff";
    msg += channel;
    msg += ",";
    msg += note;
    msg += ",";
    msg += velocity;
    ws->textAll(msg.c_str());
}

void PipoSocket::sendSensorValue(std::string axis, float value){
    if (ws == nullptr) return;
    String msg = "sensor";
    msg += axis.c_str();
    msg += ",";
    msg += value;
    ws->textAll(msg.c_str());
}

