#include "utils/debug.h"
#include "HW_CONFIG.h"
#define DEBUG_HEAP 1
void pipoDebugHeap() {
#ifdef DEBUG_HEAP

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
  Serial.printf("  Heap Fragmentation: %.2f%%\n",
                (freeHeap * 100.0) / totalHeap);
  Serial.printf(" Max block: %u bytes\n",
                heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
#endif
}