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

  //pulldown all pins
  std::set<int> nopulldown = {0, 19, 20, 26, 27, 28, 29, 30, 31, 32};
  for (int pin = 0; pin <= 48; pin++) {
    if (nopulldown.find(pin) == nopulldown.end()) {
      pinMode(pin, INPUT_PULLDOWN);
    }
  }

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // setCpuFrequencyMhz(80);  // set to 160MHz for better performance

  if (DEBUG_HEAP)
    pipoDebugHeap("Start setup");

  /////// Init hardware user interface (leds and switches)
  hwui.init();
  hwui.setup();
  //Prevent boot if battery is too low
  if (hwui.get_bat_voltage() < NO_BOOT_VOLTAGE) {
    hwui.set_led(LOW_BAT_LED, 100);
    delay(3000);
    // esp_deep_sleep_start();
    while (1) {}
  }

  /////// Init filesystem
  init_filesystem();

  /////// Load config
  Serial.print("config list:");
  Serial.println(config.get_list());
  config.load_config();

  try {
    config.apply(engine, osc, DEBUG_CONFIG);
  } catch (const std::exception& e) {
    Serial.println("failed setting conf");
  }

  /////// Init midi and hid
  String deviceName =
      "Pipo-" + String(config.general_config["PipoName"].as<String>());
  midiio.setup(deviceName.c_str());  //50k heap
#ifndef DISABLE_USB_COMM
  hidio.setup(config.general_config["HidMode"]);
#endif
  delay(1000);
  print_reset_reason();

  /////// Init wifi
  osc.init();  // Create OSC mutex before WiFi (prevents crashes from WiFi events)
  wifi.setup();  //50k heap

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  Serial.println("init sensor");
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
  Serial.println("Motion sensor button callbacks registered");
#endif

#ifdef PIPO_RANGE
  hwui.set_mode_short_press_callback([]() { input_sensor.toggle_hold_mode(); });
  Serial.println("Range sensor button callbacks registered");
#endif

  // wait for initial offsets to be measured if needed
  while (input_sensor.is_offset_measurement_complete() == false) {
    input_sensor.update();
    Serial.println("Waiting for boot offset measurement...");
  }

  Serial.println("Boot offsets measured, gather and save config");
  config.gather(engine, DEBUG_CONFIG);
  config.save(config.filename);

  if (DEBUG_HEAP)
    pipoDebugHeap();

  // Start server
  Serial.println("starting config page");
  server.setup();  // takes 30k heap

  // Configure OSC (mutex already created in init())
  osc.setup();

  if (DEBUG_HEAP)
    pipoDebugHeap();

  Serial.println("starting tasks");

  esp_task_wdt_init(2,
                    false);  // watchdog 2 seconds timeout, no panic on timeout

  //CAREFULL:
  // fileserving reports running on core 1 for now. it should be on 0
  // websocket events (not loop) reports running on core 1 for now. it should be on 0
  // should likely move button measurements in sensor task as this is similar activity
  // when engine commented, heap seems stable

  // saving increases fragmentation from 15 to 40%

  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 4096, NULL, 2,
                          &websocketTaskHandle, 0);
  xTaskCreatePinnedToCore(hwuiTask, "hwuiTask", 2048, NULL, 1, &hwuiTaskHandle,
                          0);
  xTaskCreatePinnedToCore(battmonitorTask, "battmonitorTask", 2048, NULL, 1,
                          &battmonitorTaskHandle, 0);
#ifdef PIPO_ANALOG
  xTaskCreatePinnedToCore(oscreceiveTask, "oscreceiveTask", 2048, NULL, 1,
                          &oscreceiveTaskHandle, 0);
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

  // using the main loop instead of Sensor task to optimize ram usage
  // xTaskCreatePinnedToCore(sensorTask, "sensorTask", 8000, NULL, 1,
  //                         &sensorTaskHandle, 1);  // Priority 4, Core 1, 400Hz

  Serial.println("Setup done");
}

// stack is 8k by default
// by default runs on core 1 for this board
// prio 1

void loop() {
  static bool first_run = true;
  static TickType_t xLastWakeTime;
  static const TickType_t xFrequency = pdMS_TO_TICKS(2.5);  // 400Hz max

  if (first_run) {
    xLastWakeTime = xTaskGetTickCount();
    first_run = false;
  }

  input_sensor.update();
  engine.update();

#if defined(PIPO_ANALOG) && defined(BETA_OUT)
  analog_out.update();
#endif

  vTaskDelayUntil(
      &xLastWakeTime,
      xFrequency);  // Fixed 400Hz rate  //vTaskDelay(500);  // allow task to yiedl if empty
}
