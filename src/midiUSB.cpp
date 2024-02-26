#include "midiUSB.h"

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
Adafruit_USBD_MIDI usb_midi;

MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

void midiUSBSetup() {
    TinyUSBDevice.setManufacturerDescriptor("Rup");
    TinyUSBDevice.setProductDescriptor("PipoUSB");
    MidiUsb.begin(MIDI_CHANNEL_OMNI);
    
}

void midiUSBLoop() {
    MidiUsb.read();
    MidiUsb.sendNoteOn(69, 127, 1);
    // delay(100);
    //Serial.println("Note on");
    delay(1000);
}