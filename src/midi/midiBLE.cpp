#include "midiBLE.h"

#ifdef INCLUDE_BLE

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// Newer BLE-MIDI lib allows for custom settings (and choosing lower min connection interval)
// Using generic name initially, will be updated in setup with device name from caller
BLEMIDI_CREATE_INSTANCE("Pipo-BLE", MidiBle);

void midiBLESetup(const char* deviceName) {
  // Set BLE name from passed parameter for consistency with USB, mDNS, and OSC naming
  BLEMidiBle.setName(deviceName);

  MidiBle.begin();
  BLEMidiBle.setHandleConnected(OnConnected);
  BLEMidiBle.setHandleDisconnected(OnDisconnected);

  // Start BT LED blinking to indicate BLE is enabled and waiting for connection
  hwui.start_blink(BT_LED, WIFI_AP_PULSE_TIME, 0.2);
}

void OnConnected() {
  Serial.print("BLE Connected! (Core ");
  Serial.print(xPortGetCoreID());
  Serial.println(")");
  BTconnected = true;
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void OnDisconnected() {
  Serial.print("BLE Disconnected! (Core ");
  Serial.print(xPortGetCoreID());
  Serial.println(")");
  BTconnected = false;
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MidiBLEsendCC(int control, int value, int channel) {
  MidiBle.sendControlChange(control, value, channel);
}

void MidiBLEsendNoteOn(int note, int velocity, int channel) {
  MidiBle.sendNoteOn(note, velocity, channel);
}

void MidiBLEsendNoteOff(int note, int velocity, int channel) {
  MidiBle.sendNoteOff(note, velocity, channel);
}

#endif  //INCLUDE_BLE