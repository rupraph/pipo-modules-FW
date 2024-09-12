#include "logs.h"

LittleLogs logs;
void LittleLogs::writeLog(String log, int type) {
  int date = millis();
  newLogs = true;
  allLogs += String(date) + ":" + "(" + type + ")" + log + "--";
  if (allLogs.length() > 1000) {
    allLogs = allLogs.substring(500);
  }
}
void LittleLogs::writeWarning(String log) { writeLog(log, 1); }

void LittleLogs::writeError(String log) { writeLog(log, 2); }

String LittleLogs::readLogs(bool news) {
  newLogs = false;
  int start = lastFlush;
  lastFlush = allLogs.length();
  return news ? allLogs.substring(start) : allLogs;
}

bool LittleLogs::hasNews() { return newLogs; }