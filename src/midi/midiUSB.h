#ifndef MIDIUSB_H
#define MIDIUSB_H

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "HW_CONFIG.h"
#include "utils/debug.h"

void MidiUSBSetup();
void MidiUSBsendCC(int control, int value, int channel);
void MidiUSBsendHiResCC(int value);
void MidiUSBsendNoteOn(int note, int velocity, int channel);
void MidiUSBsendNoteOff(int note, int velocity, int channel);

void MidiUSBsendPitchBend(int value, int channel);

#endif  //MIDIUSB_H