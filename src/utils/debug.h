#include <Arduino.h>
#include "shared_flags.h"
#ifndef PIPODEBUG_H
#define PIPODEBUG_H
void debug_monitor(void* pvParameters);
void pipoDebugHeapFull(const char* stepName = nullptr);
void pipoDebugHeap(const char* stepName = nullptr);
void print_reset_reason();
// Class to easily measure and report time between two points in code.

class measure_time {
 public:
  measure_time(const char* name) : name(name), last_stop_time(0) {}

  void start() {
    start_time = millis();
    // Calculate interval since last stop, if applicable
    if (last_stop_time != 0) {
      interval = start_time - last_stop_time;
    } else {
      interval = 0;
    }
  }

  void stop() {
    unsigned long end = millis();
    last_duration = end - start_time;
    last_stop_time = end;
  }

  void report() {
    Serial.print("Duration for ");
    Serial.print(name);
    Serial.print(": ");
    Serial.print(last_duration);
    Serial.print(" ms");
    Serial.print(", Interval since last stop: ");
    Serial.print(interval);
    Serial.println(" ms");
  }

 private:
  const char* name;
  unsigned long start_time = 0;
  unsigned long last_duration = 0;
  unsigned long interval = 0;
  unsigned long last_stop_time = 0;
};

extern measure_time looptime;
extern measure_time interval;
#endif  //PIPODEBUG_H