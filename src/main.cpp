#include <Arduino.h>
#include <WiFiManager.h> 
#include "fs_tools.h"
#include "midiUSB.h"
#include "midiBLE.h"


#define FORMAT_LITTLEFS_IF_FAILED true

// unsigned long t0 = millis();
// bool isConnected = false;

void setup(){
    midiUSBSetup();

    Serial.begin(115200);

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mount Success");


    WiFiManager wm;
    // reset settings - wipe stored credentials for testing
    // wm.resetSettings();
    bool res;
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

    
    midiBLESetup();


// Load config
// enable OSC if needed 
// enable BLE
// enable rtp midi
// init webserver

}

void loop() {
    midiUSBLoop();
  // read/update from sensor
  // poll webserver for config change
  // convert sensor to midi
  // send midi
}

