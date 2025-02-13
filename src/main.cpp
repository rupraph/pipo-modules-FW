
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

// quick declaration of functions
void init_filesystem();
// Tasks distribution
// what seems important is to avoid delays in midi and osc handling
// seems better to keep wifi + networking on core 0
// core 0: wifi, server, websocket
// I read contradictin info for the server/asyn tcp core. some say same as application, some say same as wifi
// core 1: sensor, midi, osc

void sensorTask(void* pvParameters) {
  for (;;) {
    input_sensor.update();
    // engine.update();
    vTaskDelay(pdMS_TO_TICKS(1));
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
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  // Disable watchdog timer for debug
  // disableCore0WDT();
  // disableCore1WDT();

  // while (!Serial)
  // delay(100);  // putting wait serial here breaks usb mid/hid init

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
  // xTaskCreatePinnedToCore(dnsTask, "dnsTask", 2048, NULL, 0, &dnsTaskHandle, 0);
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 2048, NULL, 0, &wifiTaskHandle,
                          0);
}

void loop() {
  try {

    hwui.update();

  } catch (const std::exception& e) {
    Serial.println("Exception in main loop");
    logs.writeLog(e.what());
    delay(50);
  }
  vTaskDelay(pdMS_TO_TICKS(500));
}
