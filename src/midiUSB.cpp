#include "midiUSB.h"

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);


void MidiUSBSetup() {
    TinyUSBDevice.setManufacturerDescriptor("Rup");
    TinyUSBDevice.setProductDescriptor("PipoUSB");
    MidiUsb.begin(MIDI_CHANNEL_OMNI);
    
}

void MidiUSBsendCC(int value, int control, int channel){
    MidiUsb.sendControlChange(value, control, channel);
}

void MidiUSBsendHiResCC(int value){
    int sizeddata=value & 0x3FFF;
    int msb=(sizeddata>>7) & 0x7F;
    int lsb=sizeddata & 0x7F;
    // float hsb = value >> 7;
    // float lsb = value & 127;

    MidiUsb.sendControlChange(21, msb,1);//round(floor(hsb)), 1);
    MidiUsb.sendControlChange(21+32, lsb,1);//round(ceil(lsb)), 1);

}

void MidiUSBsendNoteOn(int note, int velocity, int channel){
    MidiUsb.sendNoteOn(note, velocity, channel);
}

void MidiUSBsendNoteOff(int note, int velocity, int channel){
    MidiUsb.sendNoteOff(note, velocity, channel);
}