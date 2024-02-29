#ifndef MIDI_IO_H
#define MIDI_IO_H

#include <Arduino.h>
#include "midiUSB.h"
#include "midiBLE.h"
#include "midiRtp.h"


class midi_io
{
private:

    
public:
    midi_io();
    void midi_io_sendNoteOn(int note, int velocity, int channel);
    void midi_io_sendNoteOff(int note, int velocity, int channel);
    void midi_io_sendControlChange(int control, int value, int channel);
    void midi_io_sendHiResControlChange(int control, int value, int channel);
    void midi_io_sendProgramChange(int program, int channel);
    void midi_io_sendPitchBend(int value, int channel);
    void midi_io_sendAfterTouch(int pressure, int channel);

    // add high resoltution CC

};


#endif //MIDI_IO_H