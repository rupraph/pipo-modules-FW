#include "midiBLE.h"
#include <Arduino.h>

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32_NimBLE.h>

#ifdef PIPO_MOTION
    BLEMIDI_CREATE_INSTANCE("PipoMotioBLE", MidiBle);
#elif PIPO_RANGE
    BLEMIDI_CREATE_INSTANCE("PipoRangeBLE", MidiBle);
#elif PIPO_ANALOG
    BLEMIDI_CREATE_INSTANCE("PipoAnalogBLE", MidiBle);
#endif

void midiBLESetup() {

    MidiBle.begin();
    
    BLEMidiBle.setHandleConnected(OnConnected);
    BLEMidiBle.setHandleDisconnected(OnDisconnected);
    
    Serial.println("Midi BLE setup done");
    #ifdef DEBUG_HEAP
        Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
    #endif
}

void OnConnected() {
    Serial.println("Ble Connected!");
    hwui.set_led(BT_LED, 80);
    #ifdef DEBUG_HEAP
        Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
    #endif
}

void OnDisconnected() {
    Serial.println("Ble Disconnected!");
    hwui.set_led(BT_LED, 0);
    #ifdef DEBUG_HEAP
        Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
    #endif
}

void MidiBLEsendCC(int control, int value, int channel){
    MidiBle.sendControlChange(control, value, channel);
}

void MidiBLEsendNoteOn(int note, int velocity, int channel){
    MidiBle.sendNoteOn(note, velocity, channel);
    #ifdef DEBUG_HEAP
        Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
    #endif
}

void MidiBLEsendNoteOff(int note, int velocity, int channel){
    MidiBle.sendNoteOff(note, velocity, channel);
}

