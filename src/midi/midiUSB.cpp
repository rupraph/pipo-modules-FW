#include "midiUSB.h"

USBMIDI usb_midi;

void MidiUSBSetup() {
  USB.manufacturerName("Pipo-Interfaces");
  USB.productName("PipoUSB");
  USB.begin();
  usb_midi.begin();

  // Wait for USB to initialize
  delay(100);

  log_i("MIDI USB setup complete");
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MidiUSBsendCC(int control, int value, int channel) {
  usb_midi.controlChange(control, value, channel);
}

void MidiUSBsendHiResCC(int value) {
  int sizeddata = value & 0x3FFF;
  int msb = (sizeddata >> 7) & 0x7F;
  int lsb = sizeddata & 0x7F;
  // float hsb = value >> 7;
  // float lsb = value & 127;

  usb_midi.controlChange(21, msb, 1);       // round(floor(hsb)), 1);
  usb_midi.controlChange(21 + 32, lsb, 1);  // round(ceil(lsb)), 1);
}

void MidiUSBsendNoteOn(int note, int velocity, int channel) {
  usb_midi.noteOn(note, velocity, channel);
}

void MidiUSBsendNoteOff(int note, int velocity, int channel) {
  usb_midi.noteOff(note, velocity, channel);
}