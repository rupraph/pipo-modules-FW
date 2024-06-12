#ifndef MIDIBLE_H
#define MIDIBLE_H

#include "hw_ui.h"

void midiBLESetup();
//void midiBLELoop();
void OnConnected();
void OnDisconnected();

void MidiBLEsendCC(int control, int value, int channel);
void MidiBLEsendNoteOn(int note, int velocity, int channel);
void MidiBLEsendNoteOff(int note, int velocity, int channel);

#endif //MIDIBLE_H