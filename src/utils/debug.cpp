#include "utils/debug.h"
#include "HW_CONFIG.h"
// #define DEBUG_HEAP 1

measure_time looptime("loop");
measure_time interval("interval");

void debug_monitor(void* pvParameters) {
  for (;;) {
    // input_sensor.teleplot_data("magX");
    // input_sensor.teleplot_data("magY");
    // input_sensor.teleplot_data("magZ");
    // Serial.println(uxTaskGetStackHighWaterMark(websocketTaskHandle));
    if (DEBUG_HEAP) {
      // pipoDebugHeap("now");
      pipoDebugHeapFull("now");

      // check https://github.com/mathieucarbou/MycilaTaskMonitor
      // const UBaseType_t size = uxTaskGetStackHighWaterMark(sensorTaskHandle);
      // Serial.print("size: ");
      // Serial.println(size);
    }
    // Serial.print("Sensor task duration: ");
    // // Serial.print(sensor_task_duration);
    // Serial.print(" ms, interval: ");
    // // Serial.println(sensor_task_interval);
    looptime.report();
    // Serial.print(getCpuFrequencyMhz());
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void pipoDebugHeapFull(const char* stepName = nullptr) {

  size_t freeHeap = ESP.getFreeHeap();
  size_t totalHeap = ESP.getHeapSize();
  size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  size_t usedHeap = totalHeap - freeHeap;

  if (stepName) {
    log_d("Heap Memory Usage at %s:", stepName);
  } else {
    log_d("Heap Memory Usage:");
  }

  // Serial.printf("  Total Heap: %u bytes (%.2f KB)\n", totalHeap,
  //               totalHeap / 1024.0);
  // Serial.printf("  Used Heap:  %u bytes (%.2f KB)\n", usedHeap,
  //               usedHeap / 1024.0);
  log_d("  Free Heap:  %u bytes (%.2f KB)", freeHeap, freeHeap / 1024.0);
  // log_d(" Max block: %u bytes", largest_block);
  log_d("Fragmentation Ratio: %.2f%%",
        (1.0 - ((float)largest_block / freeHeap)) * 100);
}

void pipoDebugHeap(const char* stepName) {
  size_t freeHeap = ESP.getFreeHeap();
  if (stepName) {
    log_d("FREEHEAP at %s: %u bytes (%.2f KB)", stepName, freeHeap,
          (float)freeHeap / 1024.0f);
  } else {
    log_d("FREEHEAP: %u bytes (%.2f KB)", freeHeap, (float)freeHeap / 1024.0f);
  }
}

void print_reset_reason() {

  esp_reset_reason_t reason = esp_reset_reason();

  log_i("Reset reason: ");
  switch (reason) {
    case ESP_RST_POWERON:
      log_i("Power-on reset");
      break;
    case ESP_RST_EXT:
      log_i("External reset");
      break;
    case ESP_RST_SW:
      log_i("Software reset");
      break;
    case ESP_RST_PANIC:
      log_i("Exception/Panic reset");
      break;
    case ESP_RST_INT_WDT:
      log_i("Interrupt watchdog reset");
      break;
    case ESP_RST_TASK_WDT:
      log_i("Task watchdog reset");
      break;
    case ESP_RST_WDT:
      log_i("Other watchdog reset");
      break;
    case ESP_RST_DEEPSLEEP:
      log_i("Wakeup from deep sleep");
      break;
    case ESP_RST_BROWNOUT:
      log_i("Brownout reset");
      break;
    case ESP_RST_SDIO:
      log_i("SDIO reset");
      break;
    default:
      log_i("Unknown reset reason");
  }
}
