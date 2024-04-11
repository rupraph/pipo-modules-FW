#ifndef MIDI_IO_H
#define MIDI_IO_H

#include <Arduino.h>
#include "midiBLE.h"
#include "midiRtp.h"
#include "midiUSB.h"


class midi_io
{
private:

    
public:
    midi_io();

    bool midi_usb_enabled = true;
    bool midi_ble_connected = false;
    bool midi_rtp_connected = false;

    void sendNoteOn(int note, int velocity, int channel);
    void sendNoteOff(int note, int velocity, int channel);
    void sendControlChange(int control, int value, int channel);
    void sendHiResControlChange(int control, int value, int channel);
    void sendProgramChange(int program, int channel);
    void sendPitchBend(int value, int channel);
    void sendAfterTouch(int pressure, int channel);

    // add high resoltution CC

};


#endif //MIDI_IO_H