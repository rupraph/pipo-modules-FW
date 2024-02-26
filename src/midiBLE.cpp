#include "midiBLE.h"
#include <Arduino.h>

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

BLEMIDI_CREATE_INSTANCE("PipoMotion", MidiBle);

void OnConnected() {
    Serial.println("Ble Connected!");
}

void OnDisconnected() {
    Serial.println("Ble Disconnected!");
}

void midiBLESetup() {

    BLEMidiBle.setHandleConnected(OnConnected);
    BLEMidiBle.setHandleDisconnected(OnDisconnected);
    MidiBle.begin();
}

