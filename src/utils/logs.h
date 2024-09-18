#ifndef LITTLE_LOGS_H
#define LITTLE_LOGS_H

#include <Arduino.h>

#include "fs_tools.h"
class LittleLogs {
public:
  String allLogs;
  void writeLog(String log, int type = 0);
  void writeError(String error);
  void writeWarning(String warning);
  String readLogs(bool news = false);
  bool hasNews();

private:
  bool newLogs = false;
  bool flush = false;
  int lastFlush = 0;
};
extern LittleLogs logs;
#endif // LOGS_H