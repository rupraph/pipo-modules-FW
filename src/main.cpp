
#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "osc/osc_handler.h"
#include "server/server.h"
#include "utils/config.h"
// #include "utils/fs_tools.h"
#include "utils/logs.h"
#include "wifi/pipowifi.h"

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
PipoWifi wifi;
Engine engine;
OSC_handler osc(config);
PipoServer server(input_sens, engine, osc);

// quick declaration of functions
void init_filesystem();

// Tasks distribution
// what seems important is to avoid delays in midi and osc handling
// seems better to keep wifi + networking on core 0
// core 0: wifi, server, websocket
// I read contradictin info for the server/asyn tcp core. some say same as application, some say same as wifi
// core 1: sensor, midi, osc

TaskHandle_t sensorTaskHandle;
TaskHandle_t websocketTaskHandle;
TaskHandle_t hwuiTaskHandle;

void sensorTask(void* pvParameters) {
  for (;;) {
    input_sensor.update();
    engine.update();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void websocketTask(void* pvParameters) {
  for (;;) {
    pipoSocket.loop();
    vTaskDelay(pdMS_TO_TICKS(100));  //crashes if too fast (10 crashes)
  }
}

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
  config.apply(engine, osc, false);  // input_sens,

  /////// Init midi and hid
  midiio.setup();
  hidio.setup(config.general_config["HidMode"]);
  // while (!Serial)
  //   delay(100);

  /////// Init wifi
  wifi.setup();

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  input_sensor.init();
  input_sensor.setup();
  // capturing and storing config at this point
  //(this is a temp solution to store the initial sensor offset measurements)
  config.gather(engine, true);
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

  xTaskCreatePinnedToCore(sensorTask, "sensorTask", 8192, NULL, 1,
                          &sensorTaskHandle, 1);
  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 8192, NULL, 1,
                          &websocketTaskHandle, 0);
}

void loop() {
  try {

    // monitor_wifi(server.is_running);
    input_sens.update();
    engine.update(input_sens, midiio, hidio, osc);
    pipoSocket.loop();
    hwui.update();

    // } catch (const std::exception& e) {
    //   Serial.println("Exception in main loop");
    //   logs.writeLog(e.what());
    //   delay(50);
    // }
  }
}
