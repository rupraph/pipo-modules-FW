#include "midiBLE.h"
#include <Arduino.h>

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

BLEMIDI_CREATE_INSTANCE("PipoMotioBLE", MidiBle);

void midiBLESetup() {
    BLEMidiBle.setHandleConnected(OnConnected);
    BLEMidiBle.setHandleDisconnected(OnDisconnected);
    MidiBle.begin();
}

void OnConnected() {
    Serial.println("Ble Connected!");
    hwui.set_led(BT_LED, 80);
}

void OnDisconnected() {
    Serial.println("Ble Disconnected!");
    hwui.set_led(BT_LED, 0);
}

void MidiBLEsendCC(int control, int value, int channel){
    MidiBle.sendControlChange(control, value, channel);
}

void MidiBLEsendNoteOn(int note, int velocity, int channel){
    MidiBle.sendNoteOn(note, velocity, channel);
}

void MidiBLEsendNoteOff(int note, int velocity, int channel){
    MidiBle.sendNoteOff(note, velocity, channel);
}

