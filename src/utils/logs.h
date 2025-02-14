#ifndef LITTLE_LOGS_H
#define LITTLE_LOGS_H

#include <Arduino.h>

#include "fs_tools.h"
#define LOG_BUFFER_SIZE 1024
class LittleLogs {
 public:
  void writeLog(const String& log, int type = 0);
  void writeError(const String& error);
  void writeWarning(const String& warning);
  const char* readLogs(bool news = false);
  bool hasNews();
  int length(bool news = false);

 private:
  char allLogs[LOG_BUFFER_SIZE] = {0};
  int logIndex = 0;
  bool newLogs = false;
  bool flush = false;
  int lastFlush = 0;
  int len = 0;
};
extern LittleLogs logs;
#endif  // LOGS_H