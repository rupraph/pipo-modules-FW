#include <Arduino.h>
#ifndef PIPODEBUG_H
#define PIPODEBUG_H
void debug_monitor(void* pvParameters);
void pipoDebugHeap(const char* stepName = nullptr);
void print_reset_reason();
#endif  //PIPODEBUG_H