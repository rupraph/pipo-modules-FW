
#include <Arduino.h>
#include <WiFiManager.h>

#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "osc_handler.h"
#include "server/server.h"
#include "utils/fs_tools.h"
#include "utils/logs.h"
#include "utils/wifi_tools.h"

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

// OSC_handler osc;
midi_io midiio;
usb_hid hidio;
Engine engine(input_sens);
PipoServer server(input_sens, engine);

// quick declaration of functions
void init_filesystem();
void setup_wifi();
void monitor_wifi();

void setup() {
    Serial.begin(115200);
    // while(!Serial) // "while" prevents usb to setup properly
    // setCpuFrequencyMhz(80); will be usefull to save power on battery

    /////// Init hardware user interface (leds and switches)
    hwui.init();
    hwui.setup();

    /////// Init filesystem
    init_filesystem();

    /////// Init midi and hid
    midiio.setup();
    hidio.usb_hid_setup();

    /////// Load config
    // config.gather(input_sens, engine, false);//,
    // config.print();
    config.load_config();
    config.apply(input_sens, engine, false);  // input_sens,
    // config.print();
    /////// Init wifi
    setup_wifi();

    /////// initialize sensor/inputs
    input_sens.init();
    input_sens.setup();

    // Start server if TA connected or AP mode
    // if(WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_AP){
    Serial.println("Wifi connected, starting config page");
    server.setup();
    // }
    // else{
    //     Serial.println("Wifi not connected, no config page for now");
    // }

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
    try {
        wm.process();
        monitor_wifi(server.is_running);

        input_sens.update();

        // Measure loop time
        // Serial.print("loop");
        // Serial.println(input_sens.measured_loop);
        // Serial.print("interval");
        // Serial.println(input_sens.measured_interval_duration);

        // Plot some sensor values
        // input_sens.teleplot_data("dist");
        // input_sens.teleplot_data("roll");

        engine.update(input_sens, midiio, hidio);

        hwui.update();
    } catch (const std::exception& e) {
        Serial.println("Exception in main loop");
        logs.writeLog(e.what());
        delay(50);
    }
}
