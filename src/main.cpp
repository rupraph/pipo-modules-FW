
#include <Arduino.h>
#include <WiFiManager.h>

#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "osc/osc_handler.h"
#include "server/server.h"
#include "utils/config.h"
// #include "utils/fs_tools.h"
#include "utils/logs.h"
#include "utils/wifi_tools.h"
#include "sensors/sensors.h"

// quick declaration of functions
void init_filesystem();
void setup_wifi();
void monitor_wifi();

// Tasks distribution
// what seems important is to avoid delays in midi and osc handling
// seems better to keep wifi + networking on core 0
// core 0: wifi, server, websocket
// I read contradictin info for the server/asyn tcp core. some say same as application, some say same as wifi
// core 1: sensor, midi, osc

TaskHandle_t sensorTaskHandle;
TaskHandle_t websocketTaskHandle;
TaskHandle_t hwuiTaskHandle;
TaskHandle_t oscreceiveTaskHandle;

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

void oscreceiveTask(void* pvParameters) {
  for (;;) {
    osc.receive();
    // UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("oscreceiveTask high water mark: ");
    // Serial.println(highWaterMark);
    hw_output.update();  // should be in seperate task
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup() {
  Serial.begin(115200);
  // setCpuFrequencyMhz(80); will be usefull to save power on battery
  /////// Init hardware user interface (leds and switches)
  Serial.println(ESP.getFreeHeap());

  // hwui.init();
  // hwui.setup();
  hw_output.setup();

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
  setup_wifi();

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  input_sensor.init();
  // input_sensor.setup();

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
#ifdef ENA_OSC_OUT_TESTS
  xTaskCreatePinnedToCore(oscreceiveTask, "oscreceiveTask", 3000, NULL, 1,
                          &oscreceiveTaskHandle, 0);
#endif
}

void loop() {
  try {

    monitor_wifi(server.is_running);
    // input_sensor.update();
    // engine.update();
    // pipoSocket.loop();
    hwui.update();

    // } catch (const std::exception& e) {
    //   Serial.println("Exception in main loop");
    //   logs.writeLog(e.what());
    //   delay(50);
    // }
  } catch (const std::exception& e) {
    Serial.println("Exception in main loop");
    logs.writeLog(e.what());
    delay(50);
  }
}
