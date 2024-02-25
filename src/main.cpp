#include <Arduino.h>
#include <fs_tools.h>

#define FORMAT_LITTLEFS_IF_FAILED true

void setup(){
    Serial.begin(115200);
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mount Success");

    // init wifi
  // init webserver
  // init BLE
}

void loop() {
  // read/update from sensor
  // poll webserver for config change
  // convert sensor to midi
  // send midi
}

