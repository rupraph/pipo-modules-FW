#include "utils/system_monitor.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "utils/logs.h"
#include <vector>
#include <algorithm>

SystemMonitor systemMonitor;
TaskHandle_t SystemMonitor::monitorTaskHandle = nullptr;
bool SystemMonitor::running = false;

SystemMonitor::SystemMonitor() {}

void SystemMonitor::start(uint8_t core, uint8_t priority) {
  if (running) {
    log_w("System monitor already running");
    return;
  }

  xTaskCreatePinnedToCore(monitorTask, "sysMonitor", 4096, nullptr, priority,
                          &monitorTaskHandle, core);

  running = true;
  log_i("System monitor started on core %d", core);
}

void SystemMonitor::stop() {
  if (!running || monitorTaskHandle == nullptr) {
    return;
  }

  vTaskDelete(monitorTaskHandle);
  monitorTaskHandle = nullptr;
  running = false;
  log_i("System monitor stopped");
}

void SystemMonitor::monitorTask(void* pvParameters) {
  // System monitor is non-critical - don't subscribe to watchdog
  // to avoid false triggers during heavy I/O operations

  for (;;) {
    log_i("\n========== SYSTEM MONITOR ==========");

    printHeapStats();
    printTaskStats();
    checkStackHealth();

    log_i("====================================\n");

    vTaskDelay(pdMS_TO_TICKS(MONITOR_INTERVAL_MS));
  }
}

void SystemMonitor::report() {
  log_i("\n========== SYSTEM REPORT ==========");
  printHeapStats();
  printTaskStats();
  checkStackHealth();
  log_i("===================================\n");
}

void SystemMonitor::printHeapStats() {
  // Total heap
  size_t totalHeap = ESP.getHeapSize();
  size_t freeHeap = ESP.getFreeHeap();
  size_t usedHeap = totalHeap - freeHeap;
  float usagePercent = (float)usedHeap / totalHeap * 100.0f;

  // Largest free block (indicates fragmentation)
  size_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  float fragPercent = (1.0f - ((float)largestBlock / freeHeap)) * 100.0f;

  // Minimum free heap since boot
  size_t minFreeHeap = esp_get_minimum_free_heap_size();

  log_i("HEAP: Total=%uKB | Used=%uKB (%.1f%%) | Free=%uKB | MinFree=%uKB",
        totalHeap / 1024, usedHeap / 1024, usagePercent, freeHeap / 1024,
        minFreeHeap / 1024);
  log_i("HEAP FRAG: Largest block=%uKB (%.1f%% fragmentation)",
        largestBlock / 1024, fragPercent);

  // PSRAM stats if available
  if (psramFound()) {
    size_t totalPsram = ESP.getPsramSize();
    size_t freePsram = ESP.getFreePsram();
    size_t usedPsram = totalPsram - freePsram;
    float psramUsagePercent = (float)usedPsram / totalPsram * 100.0f;

    log_i("PSRAM: Total=%uKB | Used=%uKB (%.1f%%) | Free=%uKB",
          totalPsram / 1024, usedPsram / 1024, psramUsagePercent,
          freePsram / 1024);
  }
}

void SystemMonitor::printTaskStats() {
  // Get number of tasks
  UBaseType_t taskCount = uxTaskGetNumberOfTasks();

  // Allocate buffer for task stats (ESP-IDF 5.x uses TaskStatus_t)
  TaskStatus_t* taskStatusArray = new TaskStatus_t[taskCount];

  if (taskStatusArray == nullptr) {
    log_e("Failed to allocate memory for task stats");
    return;
  }

  // Get total run time and task stats
  uint32_t totalRunTime;
  taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);

  // Avoid division by zero
  if (totalRunTime == 0) {
    totalRunTime = 1;
  }

  log_i("TASKS: %d tasks running", taskCount);
  log_i("%-16s | Core | Prio | Stack | CPU%%  | State", "Task Name");
  log_i("---------------------------------------------------------------");

  // Sort tasks by CPU usage (optional, shows hotspots first)
  std::vector<TaskStatus_t> tasks(taskStatusArray, taskStatusArray + taskCount);
  std::sort(tasks.begin(), tasks.end(),
            [](const TaskStatus_t& a, const TaskStatus_t& b) {
              return a.ulRunTimeCounter > b.ulRunTimeCounter;
            });

  for (const auto& task : tasks) {
    // Calculate CPU percentage
    float cpuPercent = (float)task.ulRunTimeCounter / totalRunTime * 100.0f;

    // Core affinity
    const char* coreStr = "ANY";
    if (task.xCoreID == 0)
      coreStr = " 0 ";
    else if (task.xCoreID == 1)
      coreStr = " 1 ";

    // Task state
    const char* stateStr = "?";
    switch (task.eCurrentState) {
      case eRunning:
        stateStr = "RUN";
        break;
      case eReady:
        stateStr = "RDY";
        break;
      case eBlocked:
        stateStr = "BLK";
        break;
      case eSuspended:
        stateStr = "SUS";
        break;
      case eDeleted:
        stateStr = "DEL";
        break;
      default:
        stateStr = "UNK";
        break;
    }

    log_i("%-16s |  %s  |  %2u  | %5u | %4.1f%% | %s", task.pcTaskName, coreStr,
          task.uxCurrentPriority, task.usStackHighWaterMark, cpuPercent,
          stateStr);
  }

  delete[] taskStatusArray;
}

void SystemMonitor::checkStackHealth() {
  UBaseType_t taskCount = uxTaskGetNumberOfTasks();
  TaskStatus_t* taskStatusArray = new TaskStatus_t[taskCount];

  if (taskStatusArray == nullptr) {
    return;
  }

  uint32_t totalRunTime;
  taskCount = uxTaskGetSystemState(taskStatusArray, taskCount, &totalRunTime);

  bool foundIssues = false;

  for (UBaseType_t i = 0; i < taskCount; i++) {
    const char* taskName = taskStatusArray[i].pcTaskName;
    UBaseType_t stackFree = taskStatusArray[i].usStackHighWaterMark;

    // System tasks (IDLE and ipc) have smaller stacks (1024 bytes) and legitimately use 700-800 bytes
    // These are managed by ESP-IDF and their usage is normal
    bool isSystemTask =
        (strncmp(taskName, "IDLE", 4) == 0 || strncmp(taskName, "ipc", 3) == 0);

    UBaseType_t threshold = isSystemTask ? MONITOR_SYSTEM_TASK_STACK_THRESHOLD
                                         : MONITOR_STACK_WARNING_THRESHOLD;

    if (stackFree < threshold) {
      if (!foundIssues) {
        log_w("STACK WARNINGS:");
        foundIssues = true;
      }
      log_w("  %s: Only %u bytes free! (threshold: %u)%s", taskName, stackFree,
            threshold, isSystemTask ? " [system task - may be normal]" : "");
    }
  }

  if (!foundIssues) {
    log_i(
        "STACK: All tasks healthy (user tasks >%u, system tasks >%u bytes "
        "free)",
        MONITOR_STACK_WARNING_THRESHOLD, MONITOR_SYSTEM_TASK_STACK_THRESHOLD);
  }

  delete[] taskStatusArray;
}

void SystemMonitor::printCPUStats() {
  // This would require FreeRTOS trace hooks enabled
  // For now, CPU usage is shown per-task in printTaskStats()
  log_i("CPU stats integrated into task stats above");
}
