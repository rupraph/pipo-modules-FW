
#include <Arduino.h>
#include <WiFiManager.h>

#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "osc_handler.h"
#include "server/server.h"
#include "utils/config.h"
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

midi_io midiio;
usb_hid hidio;
Engine engine;
OSC_handler osc(config);
PipoServer server(input_sens, engine, osc);

// quick declaration of functions
void init_filesystem();
void setup_wifi();
void monitor_wifi();

void setup() {
  Serial.begin(115200);
  // setCpuFrequencyMhz(80); will be usefull to save power on battery
  /////// Init hardware user interface (leds and switches)
  Serial.println(ESP.getFreeHeap());
  hwui.init();
  hwui.setup();

  /////// Init filesystem
  init_filesystem();

  /////// Load config
  Serial.print("config list:");
  Serial.println(config.get_list());
  config.load_config();
  config.apply(input_sens, engine, osc, false);  // input_sens,

  /////// Init midi and hid
  midiio.setup();
  hidio.setup(config.general_config["HidMode"]);
  // while (!Serial)
  //   delay(100);

  /////// Init wifi
  setup_wifi();

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  input_sens.init();
  input_sens.setup();
  // capturing and storing config at this point
  //(this is a temp solution to store the initial sensor offset measurements)
  config.gather(input_sens, engine, true);
  config.save(config.filename);

  // Start server
  Serial.println("starting config page");
  server.setup();

  // Start OSC
  osc.setup();

  Serial.println("Setup done");
#ifdef DEBUG_HEAP
  Serial.print(F("Remaining Heap:"));
  Serial.println(String(ESP.getFreeHeap()));
  Serial.print(F("Min Free Heap:"));
  Serial.println(String(ESP.getMinFreeHeap()));
  Serial.print(F("Max Alloc Heap:"));
  Serial.println(ESP.getMaxAllocHeap());
#endif
}

void loop() {
  try {

    monitor_wifi(server.is_running);
    input_sens.update();
    engine.update(input_sens, midiio, hidio, osc);
    pipoSocket.loop();
    hwui.update();

  } catch (const std::exception& e) {
    Serial.println("Exception in main loop");
    logs.writeLog(e.what());
    delay(50);
  }
}
