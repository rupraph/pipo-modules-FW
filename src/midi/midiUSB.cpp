#include "midiUSB.h"

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

// Todo. should be carefull not to send midi too fast

void MidiUSBSetup(const char* deviceName) {
  log_i("MIDI USB setup: %s", deviceName);

  // Detach USB to change descriptors
  TinyUSBDevice.detach();
  delay(100);  // Wait for host to detect disconnect

  // Set new descriptors while detached
  TinyUSBDevice.setManufacturerDescriptor("PipoInterfaces");
  TinyUSBDevice.setProductDescriptor(deviceName);
  // Re-attach USB - this triggers re-enumeration with new descriptors
  TinyUSBDevice.attach();
  delay(100);  // Wait for re-enumeration

  // Now initialize MIDI
  MidiUsb.begin(MIDI_CHANNEL_OMNI);

  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MidiUSBsendCC(int control, int value, int channel) {
  MidiUsb.sendControlChange(control, value, channel);
}

void MidiUSBsendHiResCC(int value) {
  int sizeddata = value & 0x3FFF;
  int msb = (sizeddata >> 7) & 0x7F;
  int lsb = sizeddata & 0x7F;
  // float hsb = value >> 7;
  // float lsb = value & 127;

  MidiUsb.sendControlChange(21, msb, 1);       // round(floor(hsb)), 1);
  MidiUsb.sendControlChange(21 + 32, lsb, 1);  // round(ceil(lsb)), 1);
}

void MidiUSBsendNoteOn(int note, int velocity, int channel) {
  MidiUsb.sendNoteOn(note, velocity, channel);
}

void MidiUSBsendNoteOff(int note, int velocity, int channel) {
  MidiUsb.sendNoteOff(note, velocity, channel);
}