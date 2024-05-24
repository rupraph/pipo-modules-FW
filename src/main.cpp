




#include <Arduino.h>
#include "HW_CONFIG.h"
#include "utils/fs_tools.h"
#include <WiFiManager.h> 
#include "osc_handler.h"
#include "midi/midi_io.h"
#include "server_manager.h"
#include "engine.h"
#include "config.h"
#include "sensor/input_sensor.h"
#include "hw_ui.h"

#ifdef PIPO_MOTION
    #include "sensor/acc_sensor.h"
    MotionSensor input_sens;
    string sensor_type = "motion";
#elif defined(PIPO_RANGE)
    #include "sensor/range_sensor.h"
    RangeSensor input_sens;
    string sensor_type = "range";
#elif defined(PIPO_ANALOG)
    #include "sensor/analog_sensor.h"
    AnalogSensor input_sens;
    string sensor_type = "analog";
#endif




//OSC_handler osc;
midi_io midiio;
usb_hid hidio;
Config config;
Engine engine(input_sens);
HwUi hwui;
ServerManager server_manager(input_sens,engine,config);
WiFiManager wm;

//Todo add with wifi reconnecting attemps.
// deal with multiple identical pipo.


#define FORMAT_LITTLEFS_IF_FAILED true

// quick declaration of functions
void init_filesystem();
void setup_wifi();
void monitor_wifi();

void setup(){

    Serial.begin(115200);
    //while(!Serial) // "while" prevents usb to setup properly

    //setCpuFrequencyMhz(80);
    
    hwui.init();
    hwui.setup();
    

    init_filesystem();
    
    // Init midi and hid
    midiio.setup();
    hidio.usb_hid_setup();

    


    // Load config
    //config.gather_current_config(input_sens, engine, false);//,
    //config.print_config();
    config.load_config(sensor_type);
    config.apply_current_config(input_sens, engine,false);//input_sens,
    //config.print_config();
    
    setup_wifi();

    // initialize sensor/inputs
    input_sens.init();
    input_sens.setup();

    //Start server if TA connected or AP mode
    if(WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP){
        Serial.println("Wifi connected, starting config page");
        server_manager.setup();
        server_manager.setup_requests();
    }
    else{
        Serial.println("Wifi not connected, no config page for now");
    }
    

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

    wm.process();
    monitor_wifi();

    input_sens.update();
    // Serial.print("loop");
    // Serial.println(input_sens.measured_loop);
    // Serial.print("interval");
    // Serial.println(input_sens.measured_interval_duration);


    //input_sens.teleplot_data("T1");
    //input_sens.teleplot_data("roll");
    //input_sens.teleplot_data("dist");

    engine.update(input_sens, midiio, hidio);
    //midiio.update();

    hwui.update();
    
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
    if (config.general_config["Wifi_mode"] == "AP")
    {
        Serial.println("Starting AP mode");
        WiFi.softAP("Pipo", "pipo1234");
    }
    else{

    WiFi.mode(WIFI_STA);

    // WiFiManager wm;
    wm.setDarkMode(true);
    wm.setConfigPortalBlocking(false);
    wm.setDebugOutput(true);
    wm.setWiFiAutoReconnect(true);
    wm.setCleanConnect(true);

    if(digitalRead(MODE_SW)==LOW){
        delay(3000);
    }
    // keep pressing to reset
    if (digitalRead(MODE_SW)==LOW && digitalRead(PP_SW)==LOW)
    {
        Serial.println("Settings reset");
        wm.resetSettings();
        //Setting reset should be mover somewhere else
        // Serial.println("Launching config portal");
        // wm.setConfigPortalBlocking(true);
        // wm.autoConnect("Pipo");
        ESP.restart();
    }
    else{
        if(wm.autoConnect("Pipo")){
            Serial.println("connected...yeey :)");
            //hwui.set_led(WIFI_LED,60);
            hwui.start_pulse(WIFI_LED, 3000, 3, 30);
        }
        else {
            Serial.println("Could not connect automatically, Configportal running");
        }
    }
    }
}

void monitor_wifi(){
    // monitor wifi status
    if (WiFi.status() == WL_CONNECTED && !server_manager.is_running)
    {
        Serial.println("Wifi connected");
        hwui.start_pulse(WIFI_LED, 3000, 3, 30);

        //Todo: Starting the server here does not seem to work.
        // for now, when setting wifi for first time, then reset and server works.
        //server cannot be setup in the loop ?

        // Serial.println("Wifi connected, starting config page");
        // server_manager.setup();
        // server_manager.setup_requests();
        // wm.setDisableConfigPortal(true);
    }
    else if(WiFi.status() != WL_CONNECTED){
        //Serial.println("Wifi disconnected");
        hwui.stop_pulse(WIFI_LED);
        //server_manager.stop();
    }
}


    // connect to wifi manually
    // WiFi.mode(WIFI_STA);
    // WiFi.begin("AFC Ajax", "plompverloren567");
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // Serial.println("Connected to WiFi");
    // Serial.println(WiFi.localIP());