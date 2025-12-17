#include "midiUSB.h"
#include <esp_mac.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MidiUsb);

// Todo. should be carefull not to send midi too fast

void MidiUSBSetup(const char* deviceName) {
  log_i("MIDI USB setup: %s", deviceName);

  // Create a unique serial number by combining MAC address + device name hash
  // This forces macOS to recognize it as a different device when name changes
  uint8_t mac[6];
  esp_efuse_mac_get_default(mac);

  // Simple hash of device name to make serial unique per name
  uint32_t nameHash = 0;
  for (const char* p = deviceName; *p; p++) {
    nameHash = nameHash * 31 + *p;
  }

  // Serial: Last 3 MAC bytes (6 hex) + 16-bit hash (4 hex) = 10 chars
  char serial[12];  // Extra byte for safety
  snprintf(serial, sizeof(serial), "%02X%02X%02X%04X", mac[3], mac[4], mac[5],
           (uint16_t)(nameHash & 0xFFFF));

  log_i("Setting serial number: %s", serial);

  // Set descriptors BEFORE detaching
  TinyUSBDevice.setManufacturerDescriptor("PipoInterfaces");
  TinyUSBDevice.setProductDescriptor(deviceName);
  TinyUSBDevice.setSerialDescriptor(serial);

  // Detach and re-attach to force re-enumeration with new descriptors
  TinyUSBDevice.detach();
  delay(250);  // Wait for host to detect disconnect
  TinyUSBDevice.attach();
  delay(250);  // Wait for re-enumeration

  // Now initialize MIDI
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