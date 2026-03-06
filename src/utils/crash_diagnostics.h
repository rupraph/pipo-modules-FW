#ifndef CRASH_DIAGNOSTICS_H
#define CRASH_DIAGNOSTICS_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Initialize crash diagnostics and stack overflow detection
void initCrashDiagnostics();

// Dump all task stacks for diagnostics
void dumpAllTaskStacks();

#endif  // CRASH_DIAGNOSTICS_H
