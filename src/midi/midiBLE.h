#ifndef MIDIBLE_H
#define MIDIBLE_H

#include <Arduino.h>

#ifdef INCLUDE_BLE
#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>
#endif

#include "hw_ui.h"
#include "HW_CONFIG.h"

void midiBLESetup();
void OnConnected();
void OnDisconnected();

void MidiBLEsendCC(int control, int value, int channel);
void MidiBLEsendNoteOn(int note, int velocity, int channel);
void MidiBLEsendNoteOff(int note, int velocity, int channel);

#endif  //MIDIBLE_H