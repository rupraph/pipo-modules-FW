#include "midi_io.h"

midi_io::midi_io()
{
}

void midi_io::midi_io_sendNoteOn(int note, int velocity, int channel)
{
    //MidiUsb.sendNoteOn(note, velocity, channel);
}

void midi_io::midi_io_sendNoteOff(int note, int velocity, int channel)
{
    //MidiUsb.sendNoteOff(note, velocity, channel);
}

void midi_io::midi_io_sendControlChange(int control, int value, int channel)
{
    //MidiUsb.sendControlChange(control, value, channel);
}

void midi_io::midi_io_sendHiResControlChange(int control, int value, int channel)
{
    int sizeddata=value & 0x3FFF;
    int msb=(sizeddata>>7) & 0x7F;
    int lsb=sizeddata & 0x7F;
    // MidiUsb.sendControlChange(control, msb, channel);
    // MidiUsb.sendControlChange(control+32, lsb, channel);
    //MidiUsb.sendControlChange(control, value, channel);
}

