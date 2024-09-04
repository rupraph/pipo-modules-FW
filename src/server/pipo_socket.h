#ifndef PIPOSOCKET_H
#define PIPOSOCKET_H
#include <ESPAsyncWebServer.h>
using namespace std;

class PipoSocket
{
    public:
        PipoSocket();
        void setup(AsyncWebSocket* ws);
        void sendNoteOn(int note, int velocity, int channel);
        void sendNoteOff(int note, int velocity, int channel);
        void sendSensorValue(std::string axis, float value);

    private:
        AsyncWebSocket* ws;

};
extern PipoSocket pipoSocket;
#endif  // PIPOSOCKET_H