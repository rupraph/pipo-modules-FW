/*
 * System Monitor - Comprehensive FreeRTOS system health monitoring
 * 
 * Features:
 * - Heap usage & fragmentation tracking
 * - Per-task CPU usage (requires CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS)
 * - Stack high water marks for all tasks
 * - Task state monitoring (running/blocked/suspended)
 * - PSRAM usage (if available)
 * 
 * Usage:
 * 
 * 1. Enable in platformio.ini (already configured):
 *    -DCONFIG_FREERTOS_GENERATE_RUN_TIME_STATS=1
 *    -DCONFIG_FREERTOS_USE_TRACE_FACILITY=1
 * 
 * 2. Start automatic monitoring (reports every 10 seconds):
 *    systemMonitor.start(core, priority);  // e.g., core=1, priority=1
 * 
 * 3. Or generate one-time report anywhere in your code:
 *    SystemMonitor::report();
 * 
 * 4. Stop monitoring:
 *    systemMonitor.stop();
 * 
 * Configuration:
 * - MONITOR_INTERVAL_MS: Report interval (default 10000ms)
 * - MONITOR_STACK_WARNING_THRESHOLD: Warn if stack < threshold (default 512 bytes)
 * 
 * To enable automatic monitoring, add to platformio.ini build_flags:
 *    -DENABLE_SYSTEM_MONITOR
 */

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// System monitoring configuration
#define MONITOR_INTERVAL_MS 10000            // Report every 10 seconds
#define MONITOR_STACK_WARNING_THRESHOLD 512  // Warn if stack < 512 bytes free

class SystemMonitor {
 public:
  SystemMonitor();

  // Start the monitoring task
  void start(uint8_t core = 0, uint8_t priority = 1);

  // Stop the monitoring task
  void stop();

  // Generate a one-time report (can be called from anywhere)
  static void report();

  // Get task statistics
  static void printTaskStats();

  // Get heap statistics
  static void printHeapStats();

  // Get CPU usage per core
  static void printCPUStats();

  // Check stack health for all tasks
  static void checkStackHealth();

 private:
  static void monitorTask(void* pvParameters);
  static TaskHandle_t monitorTaskHandle;
  static bool running;
};

extern SystemMonitor systemMonitor;

#endif  // SYSTEM_MONITOR_H
