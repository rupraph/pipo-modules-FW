#include "midiUSB.h"

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

void MidiUSBSetup() {
  // Note: On ESP32, USB descriptors are set at compile time via -DUSB_MANUFACTURER and
  // -DUSB_PRODUCT in platformio.ini. These runtime calls are kept as fallback for non-ESP32.
  // TinyUSBDevice.setManufacturerDescriptor("Pipo-Interfaces");
  // TinyUSBDevice.setProductDescriptor("PipoUSB");

  // Disconnect from host before registering MIDI interface.
  // The ESP32 USB task starts before setup() runs, so the host may have
  // already enumerated CDC-only by the time we reach this call.
  // detach() forces a clean disconnect; the host re-enumerates after attach()
  // and sees the complete CDC+MIDI descriptor.
  TinyUSBDevice.detach();
  delay(
      50);  // ~50ms: enough for host to register disconnect (USB spec min: 2ms)
  MidiUsb.begin(MIDI_CHANNEL_OMNI);  // registers MIDI interface in descriptor
  TinyUSBDevice.attach();

  uint32_t timeout = millis() + 3000;
  while (!TinyUSBDevice.mounted() && millis() < timeout) {
    delay(1);
  }
  // while (!TinyUSBDevice.mounted())
  //     delay(1);
  MidiUsb.begin(MIDI_CHANNEL_OMNI);
  log_i("MIDI USB setup complete");
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

void MidiUSBsendPitchBend(int value, int channel) {
  MidiUsb.sendPitchBend(value, channel);
}