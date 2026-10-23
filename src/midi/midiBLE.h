#ifndef MIDIBLE_H
#define MIDIBLE_H

#include <Arduino.h>
#include "HW_CONFIG.h"
#include "shared_flags.h"

#ifdef INCLUDE_BLE

#include "hw_ui.h"

#include "utils/debug.h"

// BLE-MIDI transport on NimBLE. See docs/ble-midi.md.
void midiBLESetup();

// Senders only queue the message (non-blocking, safe from any task); a
// background task sends at the pace of the BLE connection. Channels are 1-16.
void MidiBLEsendCC(int control, int value, int channel);
void MidiBLEsendNoteOn(int note, int velocity, int channel);
void MidiBLEsendNoteOff(int note, int velocity, int channel);

// value is signed: -8192 to +8191
void MidiBLEsendPitchBend(int value, int channel);

// Pops the oldest Program Change received over BLE (channel 1-16).
// Non-blocking: returns false when none is pending.
bool midiBLEReadProgramChange(uint8_t* channel, uint8_t* program);

#endif
#endif  //MIDIBLE_H
