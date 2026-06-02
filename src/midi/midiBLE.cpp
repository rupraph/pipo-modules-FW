#include "midiBLE.h"

#ifdef INCLUDE_BLE

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// Newer BLE-MIDI lib allows for custom settings (and choosing lower min connection interval)

#ifdef PIPO_MOTION
BLEMIDI_CREATE_INSTANCE("PipoMotionBLE", MidiBle);
#elif PIPO_RANGE
BLEMIDI_CREATE_INSTANCE("PipoRangeBLE", MidiBle);
#elif PIPO_RANGE_L5CX
BLEMIDI_CREATE_INSTANCE("PipoRangeL5CXBLE", MidiBle);
#elif PIPO_ANALOG
BLEMIDI_CREATE_INSTANCE("PipoAnalogBLE", MidiBle);
#endif

void midiBLESetup() {
  MidiBle.begin();
  BLEMidiBle.setHandleConnected(OnConnected);
  BLEMidiBle.setHandleDisconnected(OnDisconnected);

  // Start BT LED blinking to indicate BLE is enabled and waiting for connection
  hwui.start_blink(BT_LED, WIFI_AP_PULSE_TIME, 0.2);
}

void OnConnected() {
  log_i("MIDI BLE connected");
  BTconnected = true;
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void OnDisconnected() {
  log_i("MIDI BLE disconnected");
  BTconnected = false;
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MidiBLEsendCC(int control, int value, int channel) {
  if (BTconnected) {
    MidiBle.sendControlChange(control, value, channel);
  }
}

void MidiBLEsendNoteOn(int note, int velocity, int channel) {
  if (BTconnected) {
    MidiBle.sendNoteOn(note, velocity, channel);
  }
}

void MidiBLEsendNoteOff(int note, int velocity, int channel) {
  if (BTconnected) {
    MidiBle.sendNoteOff(note, velocity, channel);
  }
}

void MidiBLEsendPitchBend(int value, int channel) {
  if (BTconnected) {
    MidiBle.sendPitchBend(value, channel);
  }
}

#endif  //INCLUDE_BLE