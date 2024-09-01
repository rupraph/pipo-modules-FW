#include <server/pipo_socket.h>

MidiSocket midisocket;
MidiSocket::MidiSocket() { 
    this->ws = nullptr;
}
void MidiSocket::setup(AsyncWebSocket* ws) { this->ws = ws; }
void MidiSocket::sendNoteOn(int note, int velocity, int channel) {
    if (ws == nullptr) return;
    String msg = "noteon";
    msg += channel;
    msg += ",";
    msg += note;
    msg += ",";
    msg += velocity;
    ws->textAll(msg.c_str());
}
void MidiSocket::sendNoteOff(int note, int velocity, int channel) {
    if (ws == nullptr) return;
    String msg = "noteoff";
    msg += channel;
    msg += ",";
    msg += note;
    msg += ",";
    msg += velocity;
    ws->textAll(msg.c_str());
}
