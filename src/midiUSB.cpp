// #include "midiUSB.h"
// #include <Arduino.h>
// #include <Adafruit_TinyUSB.h>
// #include <MIDI.h>

// Adafruit_USBD_MIDI usb_midi;

// MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

// void midiUSBSetup() {
//     TinyUSBDevice.setManufacturerDescriptor("Rup");
//     TinyUSBDevice.setProductDescriptor("PipoUSB");
//     MidiUsb.begin(MIDI_CHANNEL_OMNI);
    
// }

// void midiUSBLoop() {
//     MidiUsb.read();
//     MidiUsb.sendNoteOn(69, 127, 1);
//     // delay(100);
//     //Serial.println("Note on");
//     delay(1000);
// }

// void sendCC(int value){
//     MidiUsb.sendControlChange(1, value, 1);
// }

// void sendHiResCC(int value){
//     int sizeddata=value & 0x3FFF;
//     int msb=(sizeddata>>7) & 0x7F;
//     int lsb=sizeddata & 0x7F;
//     // float hsb = value >> 7;
//     // float lsb = value & 127;

//     MidiUsb.sendControlChange(21, msb,1);//round(floor(hsb)), 1);
//     MidiUsb.sendControlChange(21+32, lsb,1);//round(ceil(lsb)), 1);

// }

// void usbsendNoteOn(int note, int velocity, int channel){
//     MidiUsb.sendNoteOn(note, velocity, channel);
// }