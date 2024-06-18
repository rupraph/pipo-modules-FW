
#include <Arduino.h>
#include "HW_CONFIG.h"
#include "utils/fs_tools.h"
#include <WiFiManager.h> 
#include "osc_handler.h"
#include "midi/midi_io.h"
#include "server_manager.h"
#include "engine.h"
#include "utils/wifi_tools.h"

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
Engine engine(input_sens);
ServerManager server_manager(input_sens,engine);



// quick declaration of functions
void init_filesystem();
void setup_wifi();
void monitor_wifi();

void setup(){

    Serial.begin(115200);
    //while(!Serial) // "while" prevents usb to setup properly
    //setCpuFrequencyMhz(80); will be usefull to save power on battery

    /////// Init hardware user interface (leds and switches)    
    hwui.init();
    hwui.setup();

    /////// Init filesystem
    init_filesystem();
    
    /////// Init midi and hid
    midiio.setup();
    hidio.usb_hid_setup();

    /////// Load config
    //config.gather_current_config(input_sens, engine, false);//,
    //config.print_config();
    config.load_config(sensor_type);
    config.apply_current_config(input_sens, engine,false);//input_sens,
    //config.print_config();
    
    /////// Init wifi
    setup_wifi();

    /////// initialize sensor/inputs
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
    
    // Memo on tracking frequency adjustements
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


void loop() {

    wm.process();
    monitor_wifi(server_manager);

    input_sens.update();

    // Measure loop time
    // Serial.print("loop");
    // Serial.println(input_sens.measured_loop);
    // Serial.print("interval");
    // Serial.println(input_sens.measured_interval_duration);

    // Plot some sensor values
    //input_sens.teleplot_data("dist");
    input_sens.teleplot_data("roll");

    engine.update(input_sens, midiio, hidio);

    hwui.update();

}


