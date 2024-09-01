#ifndef PIPOMIDISOCKET_H
#define PIPOMIDISOCKET_H
#include <ESPAsyncWebServer.h>
using namespace std;

class MidiSocket
{
    public:
        MidiSocket();
        void setup(AsyncWebSocket* ws);
        void sendNoteOn(int note, int velocity, int channel);
        void sendNoteOff(int note, int velocity, int channel);

    private:
        AsyncWebSocket* ws;

};
extern MidiSocket midisocket;
#endif  // PIPOMIDISOCKET_H