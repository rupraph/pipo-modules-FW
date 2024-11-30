#include "utils/debug.h"
void pipoDebugHeap() {
  size_t freeHeap = ESP.getFreeHeap();
  size_t totalHeap = ESP.getHeapSize();
  size_t usedHeap = totalHeap - freeHeap;

  Serial.println("Heap Memory Usage:");
  Serial.printf("  Total Heap: %u bytes (%.2f KB)\n", totalHeap,
                totalHeap / 1024.0);
  Serial.printf("  Used Heap:  %u bytes (%.2f KB)\n", usedHeap,
                usedHeap / 1024.0);
  Serial.printf("  Free Heap:  %u bytes (%.2f KB)\n", freeHeap,
                freeHeap / 1024.0);
}