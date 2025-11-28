#include "midiUSB.h"

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

// Flow control: check buffer availability before sending
// Returns true if message was sent, false if buffer full
static bool waitForUSBBufferSpace(uint8_t bytes_needed = 4,
                                  uint32_t timeout_us = 100) {
  uint32_t start = micros();
  while (tud_midi_available() < bytes_needed) {
    if (micros() - start > timeout_us) {
#ifdef DEBUG_MIDI_BUFFER
      Serial.printf("[MIDI USB] Buffer timeout! Need %d bytes\n", bytes_needed);
#endif
      return false;  // Timeout - buffer still full
    }
    taskYIELD();  // Let USB task drain buffer
  }

#ifdef DEBUG_MIDI_BUFFER
  // Monitor buffer usage (warn if >75% full)
  uint32_t available = tud_midi_available();
  if (available > 192) {  // 192/256 = 75%
    Serial.printf("[MIDI USB] High buffer usage: %d/256 bytes\n",
                  256 - available);
  }
#endif

  return true;
}

void MidiUSBSetup() {
  TinyUSBDevice.setManufacturerDescriptor("Rup");
  TinyUSBDevice.setProductDescriptor("PipoUSB");
  // while (!TinyUSBDevice.mounted())
  //     delay(1);
  MidiUsb.begin(MIDI_CHANNEL_OMNI);
  Serial.println("Midi USB setup done");
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MidiUSBsendCC(int control, int value, int channel) {
  // Wait for buffer space with short timeout
  if (waitForUSBBufferSpace(4, 100)) {
    MidiUsb.sendControlChange(control, value, channel);
  }
  // If timeout: message dropped (shouldn't happen with 256-byte buffer)
}

void MidiUSBsendHiResCC(int value) {
  int sizeddata = value & 0x3FFF;
  int msb = (sizeddata >> 7) & 0x7F;
  int lsb = sizeddata & 0x7F;

  // Hi-res sends 2 messages, wait for space for both
  if (waitForUSBBufferSpace(8, 100)) {
    MidiUsb.sendControlChange(21, msb, 1);
    MidiUsb.sendControlChange(21 + 32, lsb, 1);
  }
}

void MidiUSBsendNoteOn(int note, int velocity, int channel) {
  if (waitForUSBBufferSpace(4, 100)) {
    MidiUsb.sendNoteOn(note, velocity, channel);
  }
}

void MidiUSBsendNoteOff(int note, int velocity, int channel) {
  if (waitForUSBBufferSpace(4, 100)) {
    MidiUsb.sendNoteOff(note, velocity, channel);
  }
}