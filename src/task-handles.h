#ifndef PIPO_TASK_HANDLES_H
#define PIPO_TASK_HANDLES_H
#include <Arduino.h>

TaskHandle_t sensorTaskHandle;
TaskHandle_t websocketTaskHandle;
TaskHandle_t hwuiTaskHandle;
TaskHandle_t buttonTaskHandle;
TaskHandle_t dnsTaskHandle;
TaskHandle_t wifiTaskHandle;
TaskHandle_t debugMonitorTaskHandle;
TaskHandle_t oscreceiveTaskHandle;
TaskHandle_t battmonitorTaskHandle;

#endif  // PIPO_TASK_HANDLES_H