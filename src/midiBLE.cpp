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

void midiBLELoop() {
    MidiBle.read();
    MidiBle.sendNoteOn(69, 127, 1);
    // delay(100);
    //Serial.println("Note on");
    delay(1000);
}


void OnConnected() {
    Serial.println("Ble Connected!");
}

void OnDisconnected() {
    Serial.println("Ble Disconnected!");
}

