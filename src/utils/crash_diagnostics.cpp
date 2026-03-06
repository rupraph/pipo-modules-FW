#include "utils/crash_diagnostics.h"
#include "utils/logs.h"
#include "esp_system.h"
#include "esp_task_wdt.h"

// FreeRTOS Stack Overflow Hook - called when stack overflow is detected
// This is a C callback required by FreeRTOS
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                              char* pcTaskName) {
  log_e("\n\n!!! STACK OVERFLOW DETECTED !!!");
  log_e("Task: %s", pcTaskName);
  log_e("Task Handle: %p", xTask);

  // Get stack info
  UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(xTask);
  log_e("Stack High Water Mark: %u bytes remaining", stackHighWaterMark);

  // Dump all task info for context
  log_e("\n=== All Task States at Crash ===");
  dumpAllTaskStacks();

  // Force a panic with detailed info
  esp_system_abort("Stack overflow");
}

// Malloc failed hook - called when heap allocation fails
extern "C" void vApplicationMallocFailedHook(void) {
  size_t freeHeap = ESP.getFreeHeap();
  size_t minFreeHeap = esp_get_minimum_free_heap_size();

  log_e("\n\n!!! MALLOC FAILED !!!");
  log_e("Free Heap: %u bytes", freeHeap);
  log_e("Minimum Free Heap: %u bytes", minFreeHeap);
  log_e("Task: %s", pcTaskGetName(NULL));

  dumpAllTaskStacks();
  esp_system_abort("Heap allocation failed");
}

void initCrashDiagnostics() {
  log_i("Crash diagnostics initialized");
  log_i("  - Stack overflow detection: ENABLED (method 2)");
  log_i("  - Heap poisoning: ENABLED");
  log_i("  - IDLE task stack: 2048 bytes");

  // Print initial state
  log_i("\n=== Initial Task Stack Status ===");
  dumpAllTaskStacks();
}

void dumpAllTaskStacks() {
  UBaseType_t taskCount = uxTaskGetNumberOfTasks();
  TaskStatus_t* taskStatusArray = new TaskStatus_t[taskCount];

  if (taskStatusArray == nullptr) {
    log_e("Failed to allocate memory for task dump");
    return;
  }

  uint32_t totalRunTime;
  taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);

  log_i("%-16s | Core | Prio | Stack Free | Status", "Task Name");
  log_i("--------------------------------------------------------");

  for (UBaseType_t i = 0; i < taskCount; i++) {
    const char* coreStr = taskStatusArray[i].xCoreID == 0   ? "  0 "
                          : taskStatusArray[i].xCoreID == 1 ? "  1 "
                                                            : " ANY";

    const char* statusStr = "OK";
    if (taskStatusArray[i].usStackHighWaterMark < 256) {
      statusStr = "CRITICAL";
    } else if (taskStatusArray[i].usStackHighWaterMark < 512) {
      statusStr = "WARNING";
    }

    log_i("%-16s |  %s |  %2u  |   %5u    | %s", taskStatusArray[i].pcTaskName,
          coreStr, taskStatusArray[i].uxCurrentPriority,
          taskStatusArray[i].usStackHighWaterMark, statusStr);
  }

  delete[] taskStatusArray;

  // Also dump heap stats
  size_t freeHeap = ESP.getFreeHeap();
  size_t minFreeHeap = esp_get_minimum_free_heap_size();
  log_i("\nHeap: Free=%uKB | MinFree=%uKB", freeHeap / 1024,
        minFreeHeap / 1024);
}
