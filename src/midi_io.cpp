#include "midi_io.h"


midi_io::midi_io()
{

}

void midi_io::setup()
{
    MidiUSBSetup();
    midiBLESetup();
    //midiRtpSetup();
}
// if sustainmil is 0 it will not send a note off
void midi_io::sendNoteOn(int note, int velocity, int channel,int sustainmil)
{
    MidiUSBsendNoteOn(note, velocity, channel);   
    MidiBLEsendNoteOn(note, velocity, channel);

    if (sustainmil>0)
    {
        active_notes.push_back(make_pair(millis()+sustainmil,note));
    }
}

void midi_io::sendNoteOff(int note, int velocity, int channel)
{
    MidiUSBsendNoteOff(note, velocity, channel);
    MidiBLEsendNoteOff(note, velocity, channel);
}

void midi_io::sendControlChange(int control, int value, int channel)
{
    MidiUSBsendCC(control, value, channel);
    MidiBLEsendCC(control, value, channel);
}

void midi_io::sendHiResControlChange(int control, int value, int channel)
{
    int sizeddata=value & 0x3FFF;
    int msb=(sizeddata>>7) & 0x7F;
    int lsb=sizeddata & 0x7F;
    // MidiUsb.sendControlChange(control, msb, channel);
    // MidiUsb.sendControlChange(control+32, lsb, channel);
    //MidiUsb.sendControlChange(control, value, channel);
}

void midi_io::update()
{
    manage_sustain();
}

void midi_io::manage_sustain()
{
    for (int i=0;i<active_notes.size();i++)
    {
        if (active_notes[i].first>millis())
        {
            sendNoteOff(active_notes[i].second, 127, 1);
            active_notes.erase(active_notes.begin()+i);
        }
    }

}

