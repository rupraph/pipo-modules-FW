#ifndef MIDI_IO_H
#define MIDI_IO_H

#include "midiBLE.h"
#include "midiRtp.h"
#include "midiUSB.h"
#include <vector>
using namespace std;

// midiio will deal with sending the midi messages to the various output interfaces
// midiio will also manage the active notes, and the sustain of the notes

class midi_io
{
private:

    
public:
    midi_io();

    bool midi_usb_enabled = true;
    bool midi_ble_connected = false;
    bool midi_rtp_connected = false;
   
    vector<pair<unsigned long,int>> active_notes; //endtime, note

    void setup();
    void sendNoteOn(int note, int velocity, int channel,int sustainmil=0);
    void sendNoteOff(int note, int velocity, int channel);
    void sendControlChange(int control, int value, int channel);
    void sendHiResControlChange(int control, int value, int channel);
    void sendProgramChange(int program, int channel);
    void sendPitchBend(int value, int channel);
    void sendAfterTouch(int pressure, int channel);
    void update();
    void manage_sustain();

    // add high resoltution CC

};


#endif //MIDI_IO_H