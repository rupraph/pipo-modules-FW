
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

// quick declaration of functions
void init_filesystem();
// Tasks distribution
// what seems important is to avoid delays in midi and osc handling
// seems better to keep wifi + networking on core 0
// core 0: wifi, server, websocket
// I read contradictin info for the server/asyn tcp core. some say same as application, some say same as wifi
// core 1: sensor, midi, osc

unsigned long last_time = 0;

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

void websocketTask(void* pvParameters) {
  for (;;) {
    if (!pipoNetworkReady()) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }

    int rssi = wifi.getRSSI();
    int taskDelay;
    // Adjust task delay based on RSSI
    if (rssi > -60) {
      taskDelay = 40;  // Strong signal → High frequency
    } else if (rssi > -70) {
      taskDelay = 100;  // Medium signal → Reduce frequency
    } else if (rssi > -80) {
      taskDelay = 250;  // Weak signal → Send less often
    } else {
      taskDelay = 500;  // Very poor signal → Minimize WebSocket activity
    }

    pipoSocket.loop();
    vTaskDelay(pdMS_TO_TICKS(taskDelay));
  }
}
void dnsTask(void* pvParameters) {
  for (;;) {
    if (!pipoNetworkReady()) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }
    // captivePortal.loop();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
void wifiTask(void* pvParameters) {
  for (;;) {
    wifi.refresh();
    vTaskDelay(pdMS_TO_TICKS(500));
    if (!server.isRunning() && wifi.ready()) {
      server.resume();
    }
  }
}

void debug_monitor(void* pvParameters) {
  for (;;) {
    // input_sensor.teleplot_data("magX");
    // input_sensor.teleplot_data("magY");
    // input_sensor.teleplot_data("magZ");
    Serial.println(uxTaskGetStackHighWaterMark(oscreceiveTaskHandle));
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

#ifdef PIPO_ANALOG
void oscreceiveTask(void* pvParameters) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED && osc.get_enabled()) {
      osc.receive();
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
// void hwuiSoftPwmTask(void* pvParameters) {
//   for (;;) {
//     hwui.update_soft_pwm();
//     vTaskDelay(pdMS_TO_TICKS(1) / 10);
//   }
// }
#endif

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  // Disable watchdog timer for debug
  // disableCore0WDT();
  // disableCore1WDT();

  while (!Serial)
    delay(100);  // putting wait serial here breaks usb mid/hid init

  // setCpuFrequencyMhz(80); will be usefull to save power on battery
  /////// Init hardware user interface (leds and switches)
  pipoDebugHeap();

  hwui.init();
  hwui.setup();
#ifdef PIPO_ANALOG
  analog_out.setup();
#endif

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
  Serial.println("init sensor");
  input_sensor.init();
  input_sensor.setup();

#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif

  // capturing and storing config at this point
  //(this is a temp solution to store the initial sensor offset measurements)
  Serial.println("gather and save config");
  config.gather(engine, DEBUG_CONFIG);
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

  // Todo move task to their own files
  xTaskCreatePinnedToCore(sensorTask, "sensorTask", 20000, NULL, 1,
                          &sensorTaskHandle, 1);
  xTaskCreatePinnedToCore(websocketTask, "websocketTask", 10000, NULL, 1,
                          &websocketTaskHandle, 0);
#ifdef PIPO_ANALOG
  xTaskCreatePinnedToCore(oscreceiveTask, "oscreceiveTask", 4096, NULL, 1,
                          &oscreceiveTaskHandle, 0);
  // xTaskCreatePinnedToCore(hwuiSoftPwmTask, "hwuiSoftPwmTask", 4096, NULL, 1,
  //                         &hwuiSoftPwmTaskHandle, 0);
#endif
  xTaskCreatePinnedToCore(dnsTask, "dnsTask", 4096, NULL, 1, &dnsTaskHandle, 0);
  // xTaskCreatePinnedToCore(debug_monitor, "debug_monitor", 4096, NULL, 1,
  //                         &debugMonitorTaskHandle, 1);
  // hwui.start_blink(WIFI_LED, 2000, 0.5);
  Serial.println("Setup done");
}

void loop() {
  try {

// hwui.update();
// I dont understand why, but the server cannot restart from a
// response to a request. It crashes. So I need to restart it from the main loop
#ifdef PIPO_ANALOG
    hwui.update_soft_pwm();
#endif

  } catch (const std::exception& e) {
    Serial.println("Exception in main loop");
    logs.writeLog(e.what());
    delay(50);
  }
}
