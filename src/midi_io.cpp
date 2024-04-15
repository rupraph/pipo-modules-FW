#include "midi_io.h"


midi_io::midi_io()
{
    //MidiUSBSetup();
    // midiBLESetup();
    // midiRtpSetup();
}

void midi_io::setup()
{
    MidiUSBSetup();
    midiBLESetup();
    //midiRtpSetup();
}

void midi_io::sendNoteOn(int note, int velocity, int channel)
{
    MidiUSBsendNoteOn(note, velocity, channel);   
    MidiBLEsendNoteOn(note, velocity, channel);
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



