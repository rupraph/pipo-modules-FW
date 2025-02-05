
#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "osc/osc_handler.h"
#include "server/server.h"
#include "utils/config.h"
#include "sensors/sensors.h"
// #include "utils/fs_tools.h"
#include "utils/logs.h"
#include "wifi/pipowifi.h"

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
TaskHandle_t dnsTaskHandle;
TaskHandle_t debugMonitorTaskHandle;

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
    vTaskDelay(pdMS_TO_TICKS(40));  //crashes if too fast (10 crashes)
  }
}
void dnsTask(void* pvParameters) {
  for (;;) {
    captivePortal.loop();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void debug_monitor(void* pvParameters) {
  for (;;) {
    // input_sensor.teleplot_data("magX");
    // input_sensor.teleplot_data("magY");
    // input_sensor.teleplot_data("magZ");
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {

  Serial.begin(115200);

  Serial.setDebugOutput(true);  // allow to print esp logs, like wifi stuff

  // Disable watchdog timer for debug
  // disableCore0WDT();
  // disableCore1WDT();

  // while (!Serial)
  //   delay(100);  // putting wait serial here breaks usb mid/hid init

  // setCpuFrequencyMhz(80); will be usefull to save power on battery
  /////// Init hardware user interface (leds and switches)
  pipoDebugHeap();

  hwui.init();
  hwui.setup();

  /////// Init filesystem
  init_filesystem();

  /////// Load config
  Serial.print("config list:");
  Serial.println(config.get_list());
  config.load_config();
  try {
    config.apply(engine, osc, DEBUG_CONFIG);  // input_sens,
  } catch (const std::exception& e) {
    Serial.println("failed setting conf");
  }

#ifndef DISABLE_USB_COMM
  /////// Init midi and hid
  midiio.setup();
  hidio.setup(config.general_config["HidMode"]);
#endif
  // while (!Serial)
  //   delay(100);
  /////// Init wifi
  wifi.setup();
  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  Serial.println("init sensor");
  input_sensor.init();
  input_sensor.setup();

#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif

  // capturing and storing config at this point
  //(this is a temp solution to store the initial sensor offset measurements)
  Serial.println("gather and save config");
  config.gather(engine, true);
  config.save(config.filename);

#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif

  // Start server
  Serial.println("starting config page");
  server.setup();
  // Start OSC
  osc.setup();

#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif

  Serial.println("starting tasks");

  xTaskCreatePinnedToCore(sensorTask, "sensorTask", 20000, NULL, 1,
                          &sensorTaskHandle, 1);
  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 10000, NULL, 1,
                          &websocketTaskHandle, 0);
  xTaskCreatePinnedToCore(dnsTask, "dnsTask", 4096, NULL, 1, &dnsTaskHandle, 0);
  // xTaskCreatePinnedToCore(debug_monitor, "debug_monitor", 4096, NULL, 1,
  //                         &debugMonitorTaskHandle, 1);

  Serial.println("Setup done");

#ifdef DISABLE_USB_COMM
  Serial.println("USB COMM DISABLED");
#endif

  // https:  //github.com/platformio/platform-espressif8266/issues/31
  // Serial.println("Testing exceptions now!!");
  // // Intentionally crash by dereferencing a null pointer
  // int* ptr = nullptr;
  // *ptr = 42;  // This will cause a crash
  // try {
  //   Serial.println("Throwing exception on purpose.");
  //   float a = 1.0 / 0.0;
  // } catch (const std::exception& e) {
  //   Serial.printf("Exception thrown: \"%s\"", e.what());
  // } catch (...) {
  //   Serial.println("Other exception thrown.");
  // }
}

void loop() {
  try {

    hwui.update();
    // I dont understand why, but the server cannot restart from a
    // response to a request. It crashes. So I need to restart it from the main loop
    if (server.should_start) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      server.start();
    }

  } catch (const std::exception& e) {
    Serial.println("Exception in main loop");
    logs.writeLog(e.what());
    delay(50);
  }
}
