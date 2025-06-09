
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
#if defined(PIPO_ANALOG)
#include "sensors/analog_out.h"
#endif

// FREERTOS core tasks distribution
// core 0: wifi, server, websocket
// core 1: sensor, midi, osc

void init_filesystem();

void setup() {  // by default on core 1

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  if (DEBUG_HEAP)
    pipoDebugHeap("Start setup");

  /////// Init hardware user interface (leds and switches)
  hwui.init();
  hwui.setup();
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
  midiio.setup();  //50k heap
#ifndef DISABLE_USB_COMM
  hidio.setup(config.general_config["HidMode"]);
#endif
  delay(1000);
  print_reset_reason();

  /////// Init wifi
  osc.setup();
  wifi.setup();  //50k heap

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  Serial.println("init sensor");
  input_sensor.init();
  input_sensor.setup();
  if (DEBUG_HEAP)
    pipoDebugHeap("End setup sensor");

  // capturing and storing config at this point
  //(temp solution to store the initial sensor offset measurements)
  Serial.println("gather and save config");
  config.gather(engine, DEBUG_CONFIG);
  config.save(config.filename);

  if (DEBUG_HEAP)
    pipoDebugHeap();

  // Start server
  Serial.println("starting config page");
  server.setup();  // takes 30k heap

  // Start OSC
  osc.setup();

  if (DEBUG_HEAP)
    pipoDebugHeap();

  Serial.println("starting tasks");

  // xTaskCreatePinnedToCore(sensorTask, "sensorTask", 5000, NULL, 1,
  // &sensorTaskHandle, 1);
  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 4096, NULL, 1,
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
  xTaskCreatePinnedToCore(
      debug_monitor, "debug_monitor", 4096, NULL, 1, &debugMonitorTaskHandle,
      1);  // for using debugheap, being on core 0 or stack 2048 causes crashes...

  hwui.start_blink(WIFI_LED, WIFI_AP_PULSE_TIME,
                   0.2);  //temporary patch to inform user pipo ready to connect
  Serial.println("Setup done");
}

// stack is 8k by default
// by default runs on core 1
void loop() {

  // #if defined(PIPO_ANALOG) && HW_REV == 10
  //   hwui.update_soft_pwm();
  // #endif

  looptime.start();
  input_sensor.update();
  engine.update();
  looptime.stop();
  // sensor_task_duration = millis() - lastMillis;
#if defined(PIPO_ANALOG) && defined(BETA_OUT)
  analog_out.update();  // should be in seperate task
#endif
  vTaskDelay(pdMS_TO_TICKS(1));

  //vTaskDelay(500);  // allow task to yiedl if empty
}
