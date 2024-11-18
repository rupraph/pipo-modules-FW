#include "logs.h"

LittleLogs logs;
void LittleLogs::writeLog(String log, int type) {
  if (allLogs.length() > 1000) {
    allLogs = "";
  }
  int date = millis();
  newLogs = true;
  allLogs += String(date) + ":" + "(" + type + ")" + log + "--";
}
void LittleLogs::writeWarning(String log) {
  writeLog(log, 1);
}

void LittleLogs::writeError(String log) {
  writeLog(log, 2);
}

String LittleLogs::readLogs(bool news) {
  newLogs = false;
  int start = lastFlush;
  lastFlush = allLogs.length();
  return news ? allLogs.substring(start) : allLogs;
}

bool LittleLogs::hasNews() {
  return newLogs;
}