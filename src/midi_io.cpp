#include "midi_io.h"

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

midi_io::midi_io()
{

}

void midi_io::setup_usb_midi()
{   
    TinyUSBDevice.setManufacturerDescriptor("Rup");
    TinyUSBDevice.setProductDescriptor("PipoUSB");
    MidiUsb.begin(MIDI_CHANNEL_OMNI);
}

void midi_io::sendNoteOn(int note, int velocity, int channel)
{
    MidiUsb.sendNoteOn(note, velocity, channel);
}

void midi_io::sendNoteOff(int note, int velocity, int channel)
{
    //MidiUsb.sendNoteOff(note, velocity, channel);
}

void midi_io::sendControlChange(int control, int value, int channel)
{
    //MidiUsb.sendControlChange(control, value, channel);
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

