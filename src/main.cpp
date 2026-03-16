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
#include "utils/system_monitor.h"
#include "utils/crash_diagnostics.h"
#include "esp_task_wdt.h"
#include <set>
#if defined(PIPO_ANALOG)
// #include "sensors/analog_out.h"
#endif

// FREERTOS core tasks distribution
// core 0: wifi, server, websocket
// core 1: sensor, midi, osc

void init_filesystem();

// In a .cpp file (like main.cpp or create stubs.cpp)
extern "C" void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                           uint8_t const* report,
                                           uint16_t len) {
  // Empty - Host mode disabled in config
}

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

  // Initialize crash diagnostics early to catch any issues
  initCrashDiagnostics();

#if defined(DISABLE_USB_COMM)
  // Wait press to start setup
  log_i("Press any key to start...");
  while (!Serial.available()) {
    delay(10);
  }
  delay(100);  // Extra time for terminal to be ready
#endif

  log_i("\n=== Pipo Setup Start ===");

  // setCpuFrequencyMhz(80);  // set to 160MHz for better performance

  if (DEBUG_HEAP)
    pipoDebugHeap("Start setup");

  /////// Init hardware user interface (leds and switches)
  hwui.init();
  hwui.setup();

  /////// Init filesystem
  init_filesystem();
  config.cleanup_temp_files();  // Clean up any orphaned temp files from crashes

  /////// Load config
  log_i("config list: %s", config.get_list().c_str());
  config.load_config();

  try {
    config.apply(engine, osc, DEBUG_CONFIG);
  } catch (const std::exception& e) {
    log_e("Failed to apply configuration: %s", e.what());
  }

  /////// Init midi and hid
  midiio.setup();  //50k heap
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
  log_i("Waiting for boot offset measurement if needed...");
  while (input_sensor.is_offset_measurement_complete() == false) {
    input_sensor.update();
  }

  log_i("Gather and save config");
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

  // Arduino 3.x auto-initializes the task watchdog - use the default configuration
  // Only the main loop task will subscribe to watchdog for critical path monitoring
#ifdef DEBUG_WATCHDOG
  log_w(
      "DEBUG_WATCHDOG: Using Arduino 3.x default task watchdog configuration");
#endif

  // Verify IDLE stack configuration
  log_i("CONFIG_FREERTOS_IDLE_TASK_STACKSIZE: %d",
        CONFIG_FREERTOS_IDLE_TASK_STACKSIZE);

  log_i("Creating tasks - monitoring heap and stack health...");
  size_t heapBefore = ESP.getFreeHeap();

  // We are using the main loop instead of a dedicated Sensor task to optimize ram usage due to arduino framework

  // Create critical tasks with error checking
  if (xTaskCreatePinnedToCore(websocketTask, "websocketTask", 3072, NULL, 2,
                              &websocketTaskHandle, 0) != pdPASS) {
    log_e("FATAL: Failed to create websocketTask - halting");
    while (1) {
      delay(1000);
    }
  }

  if (xTaskCreatePinnedToCore(hwuiTask, "hwuiTask", 2048, NULL, 1,
                              &hwuiTaskHandle, 0) != pdPASS) {
    log_e("FATAL: Failed to create hwuiTask - halting");
    while (1) {
      delay(1000);
    }
  }

  if (xTaskCreatePinnedToCore(battmonitorTask, "battmonitorTask", 4096, NULL, 1,
                              &battmonitorTaskHandle, 1) != pdPASS) {
    log_e("FATAL: Failed to create battmonitorTask - halting");
    while (1) {
      delay(1000);
    }
  }

#ifdef PIPO_ANALOG
  // xTaskCreatePinnedToCore(oscreceiveTask, "oscreceiveTask", 2048, NULL, 1,
  //                         &oscreceiveTaskHandle, 0);
// xTaskCreatePinnedToCore(hwuiSoftPwmTask, "hwuiSoftPwmTask", 4096, NULL, 1,
//                         &hwuiSoftPwmTaskHandle, 0);
#endif

  if (xTaskCreatePinnedToCore(wifiTask, "wifiTask", 4096, NULL, 3,
                              &wifiTaskHandle, 0) != pdPASS) {
    log_e("FATAL: Failed to create wifiTask - halting");
    while (1) {
      delay(1000);
    }
  }

#if HW_REV >= 11
  if (xTaskCreatePinnedToCore(buttonTask, "buttonTask", 2048, NULL, 1,
                              &buttonTaskHandle, 0) != pdPASS) {
    log_e("FATAL: Failed to create buttonTask - halting");
    while (1) {
      delay(1000);
    }
  }
#endif
  // xTaskCreatePinnedToCore(
  //     debug_monitor, "debug_monitor", 4096, NULL, 1, &debugMonitorTaskHandle,
  //     1);  // for using debugheap, being on core 0 or stack 2048 causes crashes...

  size_t heapAfter = ESP.getFreeHeap();
  log_i("All tasks created. Heap used for tasks: %d bytes",
        heapBefore - heapAfter);

  // Check stack health immediately
  log_i("\n=== Task Stack Status After Creation ===");
  dumpAllTaskStacks();

#ifdef ENABLE_SYSTEM_MONITOR
  // Start comprehensive system monitoring (reports every 10s)
  // Includes: heap stats, task CPU usage, stack health, core utilization
  systemMonitor.start(1, 1);  // Run on core 1, priority 1
  log_i("System monitor enabled - reports every 10s");
#endif

  // Generate initial system report
  log_i("\n=== Initial System State ===");
  SystemMonitor::report();

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
  // analog_out.update();
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
    log_d(
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
