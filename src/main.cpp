#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "task-handles.h"
#include "osc/osc_handler.h"
#include "server/server.h"
#include "utils/config.h"
#include "sensors/sensors.h"
#include "utils/logs.h"
#include "wifi/pipowifi.h"
#include "utils/debug.h"
#include "esp_task_wdt.h"
#include <set>
#if defined(PIPO_ANALOG)
#include "sensors/analog_out.h"
#endif

// FREERTOS core tasks distribution
// core 0: wifi, server, websocket
// core 1: sensor, midi, osc

void init_filesystem();

void setup() {  // by default on core 1

  //Pulldown all pins (avoid floating)
  std::set<int> nopulldown = {0, 19, 20, 26, 27, 28, 29, 30, 31, 32};
  for (int pin = 0; pin <= 48; pin++) {
    if (nopulldown.find(pin) == nopulldown.end()) {
      pinMode(pin, INPUT_PULLDOWN);
    }
  }

  Serial.begin(115200);
  Serial.setDebugOutput(true);

#if defined(DISABLE_USB_COMM)
  // Wait press to start setup
  log_i("Press any key to start...");
  while (!Serial.available()) {
    delay(10);
  }
  delay(100);  // Extra time for terminal to be ready
#endif

  log_i("\n=== Pipo Setup Start ===");

  config.cleanup_temp_files();  // Clean up any orphaned temp files from crashes

  // setCpuFrequencyMhz(80);  // set to 160MHz for better performance

  if (DEBUG_HEAP)
    pipoDebugHeap("Start setup");

  /////// Init hardware user interface (leds and switches)
  hwui.init();
  hwui.setup();

  /////// Init filesystem
  init_filesystem();

  /////// Load config
  log_i("config list: %s", config.get_list().c_str());
  config.load_config();

  try {
    config.apply(engine, osc, DEBUG_CONFIG);
  } catch (const std::exception& e) {
    log_e("Failed to apply configuration: %s", e.what());
  }

  /////// Init midi and hid
  String deviceName =
      "Pipo-" + String(config.general_config["PipoName"].as<String>());
  midiio.setup(deviceName.c_str());  //50k heap

#ifndef DISABLE_USB_COMM
  // hidio.setup(config.general_config["HidMode"]);
#endif
  delay(1000);
  print_reset_reason();

  /////// Init wifi
  osc.init();    // Create OSC mutex before WiFi
  wifi.setup();  //50k heap

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  log_i("Initializing sensor");
  input_sensor.init();
  input_sensor.setup();
  if (DEBUG_HEAP)
    pipoDebugHeap("End setup sensor");

    // Register button callbacks for sensor-specific actions
#ifdef PIPO_MOTION
  hwui.set_mode_short_press_callback(
      []() { input_sensor.set_new_reference_orientation(); });
  hwui.set_mode_long_press_callback(
      []() { input_sensor.toggle_relative_mode(); });
  log_i("Motion sensor button callbacks registered");
#endif

#ifdef PIPO_RANGE
  hwui.set_mode_short_press_callback([]() { input_sensor.toggle_hold_mode(); });
  log_i("Range sensor button callbacks registered");
#endif

  // wait for initial offsets to be measured if needed
  while (input_sensor.is_offset_measurement_complete() == false) {
    input_sensor.update();
    log_i("Waiting for boot offset measurement...");
  }

  log_i("Boot offsets measured, gather and save config");
  config.gather(engine, DEBUG_CONFIG);
  config.save(config.filename);

  if (DEBUG_HEAP)
    pipoDebugHeap();

  // Start web server
  log_i("Starting config page");
  server.setup();  // takes 30k heap

  // Configure OSC (mutex already created in init())
  osc.setup();

  if (DEBUG_HEAP)
    pipoDebugHeapFull();

  log_i("Starting tasks");

#ifdef DEBUG_WATCHDOG
  esp_task_wdt_init(1000, false);  // 1 second timeout in debug mode
  log_w("DEBUG_WATCHDOG enabled: 1000ms timeout");
#else
  esp_task_wdt_init(2000, false);  // 2 seconds timeout in production
#endif

  //CAREFULL:
  // fileserving reports running on core 1 for now. it should be on 0
  // websocket events (not loop) reports running on core 1 for now. it should be on 0
  // saving increases fragmentation from 15 to 40%

  // We are using the main loop instead of a dedicated Sensor task to optimize ram usage in arduino framework

  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 3072, NULL, 2,
                          &websocketTaskHandle, 0);
  xTaskCreatePinnedToCore(hwuiTask, "hwuiTask", 2048, NULL, 1, &hwuiTaskHandle,
                          0);
  xTaskCreatePinnedToCore(battmonitorTask, "battmonitorTask", 2048, NULL, 1,
                          &battmonitorTaskHandle, 0);
#ifdef PIPO_ANALOG
  // xTaskCreatePinnedToCore(oscreceiveTask, "oscreceiveTask", 2048, NULL, 1,
  //                         &oscreceiveTaskHandle, 0);
// xTaskCreatePinnedToCore(hwuiSoftPwmTask, "hwuiSoftPwmTask", 4096, NULL, 1,
//                         &hwuiSoftPwmTaskHandle, 0);
#endif
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 2048, NULL, 3, &wifiTaskHandle,
                          0);
#if HW_REV >= 11
  xTaskCreatePinnedToCore(buttonTask, "buttonTask", 2048, NULL, 1,
                          &buttonTaskHandle, 0);
#endif
  // xTaskCreatePinnedToCore(
  //     debug_monitor, "debug_monitor", 4096, NULL, 1, &debugMonitorTaskHandle,
  //     1);  // for using debugheap, being on core 0 or stack 2048 causes crashes...

  log_i("Setup complete");
}

// Loop
// stack is 8k by default
// by default runs on core 1 for this board
// prio 1
// not using a task to benefit from the already allocated heap for loop.

void loop() {
  static bool first_run = true;
  static TickType_t xLastWakeTime;
  static const TickType_t xFrequency = pdMS_TO_TICKS(2.5);  // 400Hz max
#ifdef DEBUG_WATCHDOG
  static unsigned long overrunCount = 0;
  static unsigned long lastReportTime = 0;
  static TickType_t maxExecutionTime = 0;
  static unsigned long totalExecutionTime = 0;
  static unsigned long sampleCount = 0;
#endif

  if (first_run) {
    xLastWakeTime = xTaskGetTickCount();
    esp_task_wdt_add(NULL);  // Register main loop task with watchdog
    first_run = false;
  }

#ifdef DEBUG_WATCHDOG
  TickType_t startTime = xTaskGetTickCount();
#endif

  input_sensor.update();
  engine.update();

#if defined(PIPO_ANALOG) && defined(BETA_OUT)
  analog_out.update();
#endif

#ifdef DEBUG_WATCHDOG
  TickType_t executionTime = xTaskGetTickCount() - startTime;

  // Track statistics
  if (executionTime > maxExecutionTime) {
    maxExecutionTime = executionTime;
  }
  totalExecutionTime += executionTime;
  sampleCount++;

  if (executionTime >= xFrequency) {
    overrunCount++;
  }

  if (millis() - lastReportTime > 5000) {  // Report every 5 seconds
    TickType_t avgExecutionTime =
        sampleCount > 0 ? totalExecutionTime / sampleCount : 0;
    log_w(
        "loop() stats - overruns: %lu, last: %dms, max: %dms, avg: %dms, "
        "target: %dms",
        overrunCount, pdTICKS_TO_MS(executionTime),
        pdTICKS_TO_MS(maxExecutionTime), pdTICKS_TO_MS(avgExecutionTime),
        pdTICKS_TO_MS(xFrequency));
    overrunCount = 0;
    maxExecutionTime = 0;
    totalExecutionTime = 0;
    sampleCount = 0;
    lastReportTime = millis();
  }
#endif

  esp_task_wdt_reset();  // Reset watchdog in main loop

  vTaskDelayUntil(
      &xLastWakeTime,
      xFrequency);  // Fixed 400Hz rate // allow task to yiedl if empty
}
