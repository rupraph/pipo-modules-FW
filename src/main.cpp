
#include "HW_CONFIG.h"
#include "engine.h"
#include "hw_ui.h"
#include "midi/midi_io.h"
#include "task-handles.h"
#include "osc/osc_handler.h"
#include "server/server.h"
#include "utils/config.h"
#include "sensors/sensors.h"
// #include "utils/fs_tools.h"
#include "utils/logs.h"
#include "wifi/pipowifi.h"

#ifdef PIPO_ANALOG
#include "sensors/analog_out.h"
#endif

// Tasks distribution
// wifi + networking on core 0
// core 0: wifi, server, websocket
// core 1: sensor, midi, osc
// #define ASYNC_TCP_RUNNING_CORE 0

void init_filesystem();

void sensorTask(void* pvParameters) {
  for (;;) {
    input_sensor.update();
    engine.update();
    hwui.update();
#ifdef PIPO_ANALOG
    analog_out.update();  // should be in seperate task
#endif
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

// takes 2-3 ms for motion
void websocketTask(void* pvParameters) {
  for (;;) {
    if (!pipoNetworkReady()) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }
    int rssi = wifi.getRSSI();
    int taskDelay;
    // Adjust task delay based on RSSI
    if (rssi > -65) {
      taskDelay = 40;  // Strong signal → High frequency
    } else if (rssi > -70) {
      taskDelay = 80;  // Medium signal → Reduce frequency
    } else if (rssi > -80) {
      taskDelay = 250;  // Weak signal → Send less often
    } else {
      taskDelay = 500;  // Very poor signal → Minimize WebSocket activity
    }

    pipoSocket.loop();
    vTaskDelay(pdMS_TO_TICKS(taskDelay));
  }
}
void wifiTask(void* pvParameters) {
  for (;;) {
    wifi.refresh();
    vTaskDelay(pdMS_TO_TICKS(500));
    if (wifi.ready()) {
      if (!server.isRunning()) {
        server.resume();
      }
      if (osc.is_enabled()) {
        osc.start();
      }
    }
  }
}

void debug_monitor(void* pvParameters) {
  for (;;) {
    // input_sensor.teleplot_data("magX");
    // input_sensor.teleplot_data("magY");
    // input_sensor.teleplot_data("magZ");
    // Serial.println(uxTaskGetStackHighWaterMark(websocketTaskHandle));
    if (DEBUG_HEAP)
      pipoDebugHeap();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#ifdef PIPO_ANALOG
void oscreceiveTask(void* pvParameters) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED && osc.is_enabled()) {
      osc.receive();
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
// hwuiSoftPwmTask temporarily in the main loop. not smooth when in task
// void hwuiSoftPwmTask(void* pvParameters) {
//   for (;;) {
//     hwui.update_soft_pwm();
//     vTaskDelay(pdMS_TO_TICKS(1) / 10);
//   }
// }
#endif

// by default runs on core 1
void setup() {
  delay(2000);
  Serial.begin(115200);
  delay(2000);
  Serial.setDebugOutput(true);
  print_reset_reason();
  if (DEBUG_HEAP)
    pipoDebugHeap("Start setup");
  // Free Heap : 215k (total 277k)
  delay(2000);

  while (!Serial)
    delay(100);  // putting wait serial here breaks usb mid/hid init

  // setCpuFrequencyMhz(80); will be usefull to save power on battery

  /////// Init hardware user interface (leds and switches)
  hwui.init();
  hwui.setup();
  //Free Heap : 214k

#ifdef PIPO_ANALOG
  analog_out.setup();
#endif

  /////// Init filesystem
  init_filesystem();
  //Free Heap : 212k

  /////// Load config
  Serial.print("config list:");
  Serial.println(config.get_list());
  config.load_config();
  //Free Heap : 205k
  try {
    config.apply(engine, osc, DEBUG_CONFIG);  // input_sens,
  } catch (const std::exception& e) {
    Serial.println("failed setting conf");
  }

  /////// Init midi and hid
  midiio.setup();
  //Free Heap : 156k
#ifndef DISABLE_USB_COMM
  hidio.setup(config.general_config["HidMode"]);
#endif
  //Free Heap : 156k
  /////// Init wifi
  osc.setup();
  //Free Heap : 156k
  wifi.setup();
  //Free Heap : 101k

  /////// print filesystem files list
  listDir(LittleFS, "/", 0);

  /////// initialize sensor/inputs
  Serial.println("init sensor");
  input_sensor.init();
  input_sensor.setup();
  if (DEBUG_HEAP)
    pipoDebugHeap("End setup sensor");
  //Free Heap : 98k

  // capturing and storing config at this point
  //(this is a temp solution to store the initial sensor offset measurements)
  Serial.println("gather and save config");
  config.gather(engine, DEBUG_CONFIG);
  config.save(config.filename);

  if (DEBUG_HEAP)
    pipoDebugHeap();
  //Free Heap : 99k

  // Start server
  Serial.println("starting config page");
  server.setup();
  //Free Heap : 71k

  // Start OSC
  osc.setup();
  //Free Heap : 71k

  if (DEBUG_HEAP)
    pipoDebugHeap();

  Serial.println("starting tasks");

  // Todo move task to their own files
  if (DEBUG_HEAP)
    pipoDebugHeap();
  xTaskCreatePinnedToCore(sensorTask, "sensorTask", 5000, NULL, 1,
                          &sensorTaskHandle, 1);
  if (DEBUG_HEAP)
    pipoDebugHeap();
  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 4096, NULL, 1,
                          &websocketTaskHandle, 0);
  if (DEBUG_HEAP)
    pipoDebugHeap();
    //Free Heap : 61k
#ifdef PIPO_ANALOG
  xTaskCreatePinnedToCore(oscreceiveTask, "oscreceiveTask", 4096, NULL, 1,
                          &oscreceiveTaskHandle, 0);
  // xTaskCreatePinnedToCore(hwuiSoftPwmTask, "hwuiSoftPwmTask", 4096, NULL, 1,
  //                         &hwuiSoftPwmTaskHandle, 0);
#endif
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 2048, NULL, 1, &wifiTaskHandle,
                          0);
  // xTaskCreatePinnedToCore(debug_monitor, "debug_monitor", 4096, NULL, 1,
  //                         &debugMonitorTaskHandle, 1);
  hwui.start_blink(WIFI_LED, WIFI_AP_PULSE_TIME,
                   0.2);  //temporary patch to inform user pipo ready to connect
  Serial.println("Setup done");
}

// by default runs on core 1
void loop() {
  try {
#ifdef PIPO_ANALOG
    hwui.update_soft_pwm();
#endif
  } catch (const std::exception& e) {
    Serial.println("Exception in main loop");
    logs.writeLog(e.what());
    delay(50);
  }
}
