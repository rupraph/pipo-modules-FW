#include <Arduino.h>
#include "fs_tools.h"
#include <WiFiManager.h> 
#include "midiUSB.h"
#include "midiBLE.h"
#include "midiRtp.h"
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

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
    delay(2000);

    
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }



    listDir(LittleFS, "/", 2);
    
    midiBLESetup();
    midiRtpSetup(); //-> not working. can't see the device from mac or windows

    //server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    //server.begin();
    

// Load config
// enable OSC if needed 
// enable rtp midi


}

void loop() {
    midiUSBLoop();
    midiRtpLoop();
    midiBLELoop();
  // read/update from sensor
  // poll webserver for config change
  // convert sensor to midi
  // send midi
}


    // connect to wifi manually
    // WiFi.mode(WIFI_STA);
    // WiFi.begin("Klurp", "plokplokplok");
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("Connected to WiFi");
    // Serial.println(WiFi.localIP());