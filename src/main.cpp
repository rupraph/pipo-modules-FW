#include <Arduino.h>
#include "fs_tools.h"
#include <WiFiManager.h> 
#include "osc_handler.h"
#include "midi_io.h"
#include "acc_sensor.h"
#include "server_manager.h"
#include "engine.h"
#include "config.h"


//OSC_handler osc;
midi_io midiio;
usb_hid hidio;
Config config;
Engine engine;
ServerManager server_manager(engine,config);


#define FORMAT_LITTLEFS_IF_FAILED true

// unsigned long t0 = millis();
// bool isConnected = false;

void setup(){
    sensor& acc_sensor = sensor::getInstance();

    Serial.begin(115200);
    //while(!Serial) // while prevent usb to setup properly



    // Init LittleFS
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
    }
    Serial.println("LittleFS Mount Success");
    // listDir(LittleFS, "/config", 2);
    // listDir(LittleFS, "/webpage", 2);

    // Load config
    config.load_config_from_file("/config/current_config.json");
    config.apply_current_config(acc_sensor,engine,true);

    //config.gather_current_config(acc_sensor,engine,true);
    //config.save_config("/config/current_config.json");

    // Init midi
    midiio.setup();
    hidio.usb_hid_setup();

    //Init Wifi 
    WiFiManager wm;
    // reset settings - wipe stored credentials for testing
    if(digitalRead(35)==HIGH)
    {
        wm.resetSettings();
        Serial.println("Settings reset");}
    bool res;
    wm.setDebugOutput(true);
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap
    delay(2000);
        if(!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }


    // engine.Miditranslators["roll"].set_param("translator_mode", 0);


    // Initialize the ICM-20948
    Wire.begin(2, 1, 400000);
    acc_sensor.init();
    acc_sensor.setup();

    // check that wifi is connected
    server_manager.setup();
    server_manager.setup_requests();
    
}

    // Initialize OSC
    // osc.setDestIp(IPAddress(172,20,10,14));
    // osc.setoutPort(8000);
    // osc.start();

void loop() {
    sensor& acc_sensor = sensor::getInstance();
    acc_sensor.update();


    engine.update(midiio, hidio);
    midiio.update();
    
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