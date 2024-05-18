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
   
    // should have as many active notes vector as channels
    vector<pair<unsigned long,int>> active_notes; //sustained endtime, note
    uint8_t lastnote_on[16];
    uint8_t lastnote_off[16];
    
    // this should be replaced by dynamic allocation since this takes a significant amount of memory
    uint8_t lastcc[16][128];

    bool off_before_next_note = false; // sends off previous note before sending note

    void setup();
    void sendNoteOn(int note, int velocity, int channel,unsigned long sustainmil=0);
    void sendNoteOff(int note, int velocity, int channel);
    void all_notes_off();
    void sendControlChange(int control, int value, int channel, bool hires=false);
    void sendProgramChange(int program, int channel);
    void sendPitchBend(int value, int channel);
    void sendAfterTouch(int pressure, int channel);
    void update();
    
    private:
    void sendHiResControlChange(int control, int value, int channel);
    void manage_sustain();
};


#endif //MIDI_IO_H