




#include <Arduino.h>
#include "HW_CONFIG.h"
#include "fs_tools.h"
#include <WiFiManager.h> 
#include "osc_handler.h"
#include "midi_io.h"
#include "server_manager.h"
#include "engine.h"
#include "config.h"
#include "input_sensor.h"
#include "hw_ui.h"

#ifdef PIPO_MOTION
    #include "acc_sensor.h"
    MotionSensor input_sens;
    string sensor_type = "motion";
#elif defined(PIPO_RANGE)
    #include "range_sensor.h"
    RangeSensor input_sens;
    string sensor_type = "range";
#elif defined(PIPO_ANALOG)
    #include "analog_sensor.h"
    AnalogSensor input_sens;
    string sensor_type = "analog";
#endif




//OSC_handler osc;
midi_io midiio;
usb_hid hidio;
Config config;
Engine engine;
HwUi hwui;
ServerManager server_manager(input_sens,engine,config);


#define FORMAT_LITTLEFS_IF_FAILED true

// quick declaration of functions
void init_filesystem();
void setup_wifi();

void setup(){

    Serial.begin(115200);
    //while(!Serial) // "while" prevents usb to setup properly

    //setCpuFrequencyMhz(80);
    
    hwui.init();


    init_filesystem();
    
    // Init midi and hid
    midiio.setup();
    hidio.usb_hid_setup();

    setup_wifi();

    // Load config
    //config.gather_current_config(input_sens, engine, false);//,
    //config.print_config();
    config.load_config(sensor_type);
    config.apply_current_config(input_sens, engine,false);//input_sens,
    //config.print_config();
    
    // initialize sensor/inputs
    input_sens.init();
    input_sens.setup();

    //Start server
    // Todo: add check that wifi is connected before running server...
    server_manager.setup();
    server_manager.setup_requests();

    // uint32_t Freq = getCpuFrequencyMhz();
    // Serial.print("CPU Freq = ");
    // Serial.print(Freq);
    // Serial.println(" MHz");
    // Freq = getXtalFrequencyMhz();
    // Serial.print("XTAL Freq = ");
    // Serial.print(Freq);
    // Serial.println(" MHz");
    // Freq = getApbFrequency();
    // Serial.print("APB Freq = ");
    // Serial.print(Freq);
    // Serial.println(" Hz");

    Serial.println("Setup done");
}

    // Initialize OSC
    // osc.setDestIp(IPAddress(172,20,10,14));
    // osc.setoutPort(8000);
    // osc.start();

void loop() {

    // should create a task to trigger sensor and engine computation at regular interval



    input_sens.update();
    input_sens.teleplot_data("T1");
    //input_sens.teleplot_data("roll");
    //input_sens.teleplot_data("dist");

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


void init_filesystem(){
        // Init LittleFS
        if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
        }
        Serial.println("LittleFS Mount Success");
        // listDir(LittleFS, "/config", 2);
        // listDir(LittleFS, "/webpage", 2);
}

void setup_wifi(){
    // setup wifi through wifi manager

    WiFiManager wm;

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
        hwui.set_led(0,122);
        hwui.set_led(1,255);


    }
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