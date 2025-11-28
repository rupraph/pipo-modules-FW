#include "midiBLE.h"

#ifdef INCLUDE_BLE

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

// Newer BLE-MIDI lib allows for custom settings (and choosing lower min connection interval)

#ifdef PIPO_MOTION
BLEMIDI_CREATE_INSTANCE("PipoMotionBLE", MidiBle);
#elif PIPO_RANGE
BLEMIDI_CREATE_INSTANCE("PipoRangeBLE", MidiBle);
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
  Serial.println("Ble Connected!");
  BTconnected = true;

  // Request faster connection interval for low-latency MIDI
  // Default is 30-50ms, we request 7.5-15ms for real-time performance
  auto server = NimBLEDevice::getServer();
  if (server && server->getConnectedCount() > 0) {
    // Get the most recent connection
    auto connInfo = server->getPeerInfo(0);
    uint16_t conn_handle = connInfo.getConnHandle();

    // Connection parameters (all in units):
    // minInterval: 6 units = 7.5ms (6 * 1.25ms)
    // maxInterval: 12 units = 15ms (12 * 1.25ms)
    // latency: 0 (no latency tolerance)
    // timeout: 400 units = 4000ms (400 * 10ms)
    server->updateConnParams(conn_handle, 6, 12, 0, 400);

    Serial.println("BLE: Requested 7.5-15ms connection interval");
  }

  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void OnDisconnected() {
  Serial.println("Ble Disconnected!");
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

void MidiBLEbeginBatch() {
  // Begin a batch transmission - messages will accumulate in the transport buffer
  // until MidiBLEendBatch() is called, allowing multiple MIDI messages
  // to be sent in a single BLE packet for improved performance
  BLEMidiBle.beginTransmission(MIDI_NAMESPACE::InvalidType);
}

void MidiBLEendBatch() {
  // Flush all accumulated MIDI messages in one BLE packet
  BLEMidiBle.endTransmission();
}

#endif  //INCLUDE_BLE