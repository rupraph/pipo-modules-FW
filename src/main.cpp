

#define PIPO_RANGE

#include <Arduino.h>
#include "fs_tools.h"
#include <WiFiManager.h> 
#include "osc_handler.h"
#include "midi_io.h"
#include "server_manager.h"
#include "engine.h"
#include "config.h"
#include "input_sensor.h"

#ifdef PIPO_MOTION
    #include "acc_sensor.h"
    MotionSensor input_sens;
#elif defined(PIPO_RANGE)
    #include "range_sensor.h"
    RangeSensor input_sens;
// #elif defined(PIPO_ANALOG)
//     analog_sensor input_sens;
#endif

//OSC_handler osc;
midi_io midiio;
usb_hid hidio;
Config config;
Engine engine;
ServerManager server_manager(input_sens,engine,config);


#define FORMAT_LITTLEFS_IF_FAILED true


void setup(){

    Serial.begin(115200);
    //while(!Serial) // "while" prevents usb to setup properly


    // Init LittleFS
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("LittleFS Mount Failed");
    return;
    }
    Serial.println("LittleFS Mount Success");
    // listDir(LittleFS, "/config", 2);
    // listDir(LittleFS, "/webpage", 2);


    
    // Init midi and hid
    midiio.setup();
    hidio.usb_hid_setup();

    //Init Wifi 
    WiFiManager wm;
    // reset settings - wipe stored credentials for testing
    if(digitalRead(35)==HIGH){
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
        Serial.println("connected...yeey :)");
    }


    // Load config
    config.gather_current_config(input_sens, engine, false);//,
    config.print_config();
    
    #if defined(PIPO_MOTION)
        config.load_config_from_file("/config/motion_config.json");
    #elif defined(PIPO_RANGE)
        config.load_config_from_file("/config/range_config.json");
    #elif defined(PIPO_ANALOG)
        config.load_config_from_file("/config/analog_config.json");
    #endif

    config.apply_current_config(input_sens, engine,false);//input_sens,
    // config.gather_current_config(input_sens,engine,false);
    config.print_config();

    // save config
    // config.gather_current_config(input_sens,engine,true);
    // config.save_config("/config/current_config.json");
    // config.print_config();


    // Initialize the ICM-20948
    Wire.begin(2, 1, 400000);
    
    input_sens.init();
    input_sens.setup();

    // check that wifi is connected
    server_manager.setup();
    server_manager.setup_requests();

    Serial.println("Setup done");
    
}

    // Initialize OSC
    // osc.setDestIp(IPAddress(172,20,10,14));
    // osc.setoutPort(8000);
    // osc.start();

void loop() {

    // should create a task to trigger sensor and engine computation at regular interval

    Serial.println("loop");

    //input_sens.enable_send_vizualizer = true;
    input_sens.update();
    //input_sens.teleplot_data("roll");
    //input_sens.teleplot_data("dist");
    // input_sens.teleplot_data("yaw");

    engine.update(input_sens, midiio, hidio);
    midiio.update();
    
    // osc.sendOscMessage(input_sens.roll);
    // osc.sendOscMessage(input_sens.pitch);
    // osc.sendOscMessage(input_sens.yaw);
    
    
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