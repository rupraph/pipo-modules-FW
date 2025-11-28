#ifndef MIDIBLE_H
#define MIDIBLE_H

#include <Arduino.h>
#include "HW_CONFIG.h"
#include "shared_flags.h"

#ifdef INCLUDE_BLE

#include "hw_ui.h"

#include "utils/debug.h"

// Custom BLE-MIDI buffer size for high-throughput applications
// Default is 64 bytes, we increase to 128 for burst handling
#define BLEMIDI_NAMESPACE BLEMidi
namespace BLEMIDI_NAMESPACE {
struct CustomBLESettings {
  static const size_t MaxBufferSize = 128;
};
}  // namespace BLEMIDI_NAMESPACE
#define DefaultSettings CustomBLESettings

void midiBLESetup();
void OnConnected();
void OnDisconnected();

void MidiBLEsendCC(int control, int value, int channel);
void MidiBLEsendNoteOn(int note, int velocity, int channel);
void MidiBLEsendNoteOff(int note, int velocity, int channel);

// Batch sending for improved BLE performance
void MidiBLEbeginBatch();
void MidiBLEendBatch();

#endif
#endif  //MIDIBLE_H