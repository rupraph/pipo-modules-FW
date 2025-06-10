#include "utils/debug.h"
#include "HW_CONFIG.h"
// #define DEBUG_HEAP 1

measure_time looptime("loop");

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
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void pipoDebugHeapFull(const char* stepName = nullptr) {

  size_t freeHeap = ESP.getFreeHeap();
  size_t totalHeap = ESP.getHeapSize();
  size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  size_t usedHeap = totalHeap - freeHeap;

  if (stepName) {
    Serial.printf("Heap Memory Usage at %s:\n", stepName);
  } else {
    Serial.println("Heap Memory Usage:");
  }

  // Serial.printf("  Total Heap: %u bytes (%.2f KB)\n", totalHeap,
  //               totalHeap / 1024.0);
  // Serial.printf("  Used Heap:  %u bytes (%.2f KB)\n", usedHeap,
  //               usedHeap / 1024.0);
  Serial.printf("  Free Heap:  %u bytes (%.2f KB)\n", freeHeap,
                freeHeap / 1024.0);
  Serial.printf(" Max block: %u bytes\n", largest_block);
  Serial.printf("Fragmentation Ratio: %.2f%%\n",
                (1.0 - ((float)largest_block / freeHeap)) * 100);
}

void pipoDebugHeap(const char* stepName = nullptr) {
  size_t freeHeap = ESP.getFreeHeap();
  if (stepName) {
    Serial.printf("FREEHEAP at %s:", stepName);
  } else {
    Serial.println("FREEHEAP:");
  }
  Serial.printf("  Free Heap:  %u bytes (%.2f KB)\n", freeHeap,
                freeHeap / 1024.0);
}

void print_reset_reason() {

  esp_reset_reason_t reason = esp_reset_reason();

  Serial.print("Reset reason: ");
  switch (reason) {
    case ESP_RST_POWERON:
      Serial.println("Power-on reset");
      break;
    case ESP_RST_EXT:
      Serial.println("External reset");
      break;
    case ESP_RST_SW:
      Serial.println("Software reset");
      break;
    case ESP_RST_PANIC:
      Serial.println("Exception/Panic reset");
      break;
    case ESP_RST_INT_WDT:
      Serial.println("Interrupt watchdog reset");
      break;
    case ESP_RST_TASK_WDT:
      Serial.println("Task watchdog reset");
      break;
    case ESP_RST_WDT:
      Serial.println("Other watchdog reset");
      break;
    case ESP_RST_DEEPSLEEP:
      Serial.println("Wakeup from deep sleep");
      break;
    case ESP_RST_BROWNOUT:
      Serial.println("Brownout reset");
      break;
    case ESP_RST_SDIO:
      Serial.println("SDIO reset");
      break;
    default:
      Serial.println("Unknown reset reason");
  }
}
