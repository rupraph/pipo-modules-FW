#ifndef MIDIBLE_H
#define MIDIBLE_H

#include <Arduino.h>
#include "HW_CONFIG.h"

#ifdef INCLUDE_BLE

#include "hw_ui.h"

#include "utils/debug.h"

void midiBLESetup();
void OnConnected();
void OnDisconnected();

void MidiBLEsendCC(int control, int value, int channel);
void MidiBLEsendNoteOn(int note, int velocity, int channel);
void MidiBLEsendNoteOff(int note, int velocity, int channel);

#endif
#endif  //MIDIBLE_H