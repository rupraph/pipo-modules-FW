#include <Arduino.h>
#include "fs_tools.h"
#include <WiFiManager.h> 
#include "osc_handler.h"
#include "midi_io.h"
#include "acc_sensor.h"
#include "server_manager.h"
#include "engine.h"

OSC_handler osc;
MidiTranslator midi_translator;
midi_io midiio; //causes crashes as of now


#define FORMAT_LITTLEFS_IF_FAILED true

// unsigned long t0 = millis();
// bool isConnected = false;

void setup(){
    sensor& acc_sensor = sensor::getInstance();
    //MidiUSBSetup();
    midiio.setup();

    Serial.begin(115200);

    // if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    //     Serial.println("LittleFS Mount Failed");
    //     return;
    // }
    // Serial.println("LittleFS Mount Success");
    //listDir(LittleFS, "/", 2);

    //////////// Wifi 
    // WiFiManager wm;
    // // reset settings - wipe stored credentials for testing
    // // wm.resetSettings();
    // bool res;
    // //wm.setDebugOutput(true);
    // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    // delay(2000);
    //     if(!res) {
    //     Serial.println("Failed to connect");
    //     // ESP.restart();
    // } 
    // else {
    //     //if you get here you have connected to the WiFi    
    //     Serial.println("connected...yeey :)");
    // }

    //connect to wifi manually
    // WiFi.mode(WIFI_STA);
    // WiFi.begin("freebox_RZWVFD", "AZERTYUIOP");
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("Connected to WiFi");
    // Serial.println(WiFi.localIP());

    

    

    Wire.begin(2, 1, 400000);
    acc_sensor.init();
    acc_sensor.setup();

    // osc.setDestIp(IPAddress(172,20,10,14));
    // osc.setoutPort(8000);
    // osc.start();

    // midi_translator.set_Scale_Type("minor");
    // midi_translator.printScale(midi_translator.current_scale);


}



void loop() {
    sensor& acc_sensor = sensor::getInstance();
    acc_sensor.update();

    midiio.sendControlChange(1, acc_sensor.data_map["roll"], 1);

    
    // osc.sendOscMessage(acc_sensor.roll);
    // osc.sendOscMessage(acc_sensor.pitch);
    // osc.sendOscMessage(acc_sensor.yaw);
    
    
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