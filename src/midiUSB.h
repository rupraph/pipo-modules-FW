#ifndef MIDIUSB_H
#define MIDIUSB_H



void midiUSBSetup();
void midiUSBLoop();
void sendCC(int value);
void sendHiResCC(int value);

#endif //MIDIUSB_H