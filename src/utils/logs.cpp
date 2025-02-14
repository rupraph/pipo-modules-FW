#include "logs.h"

LittleLogs logs;

void LittleLogs::writeLog(const String& log, int type) {
  if (logIndex > LOG_BUFFER_SIZE - 128) {  // Prevent overflow
    logIndex = 0;                          // Reset index
    memset(allLogs, 0, LOG_BUFFER_SIZE);   // Clear buffer
  }
  int date = millis();
  newLogs = true;

  char logEntry[128];  // Fixed-size buffer to prevent heap fragmentation
  snprintf(logEntry, sizeof(logEntry), "%d:(%d)%s--", date, type, log.c_str());
  strncat(allLogs, logEntry, LOG_BUFFER_SIZE - logIndex - 1);
  logIndex = strlen(allLogs);
}

void LittleLogs::writeWarning(const String& log) {
  writeLog(log, 1);
}

void LittleLogs::writeError(const String& log) {
  writeLog(log, 2);
}

const char* LittleLogs::readLogs(bool news) {
  newLogs = false;
  int start = lastFlush;
  lastFlush = logIndex;
  return news ? &allLogs[start] : allLogs;
}

bool LittleLogs::hasNews() {
  return newLogs;
}

int LittleLogs::length(bool news) {
  return news ? logIndex - lastFlush : logIndex;
}
