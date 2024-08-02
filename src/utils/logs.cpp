#include "logs.h"

LittleLogs logs;
void LittleLogs::writeLog(String log, int type) {
    int date = millis();
    allLogs += String(date) + ":" +"(" + type + ")" + log + "\n";
    if (allLogs.length() > 1000) {
        allLogs = allLogs.substring(500);
    }
}
void LittleLogs::writeWarning(String log) {
    writeLog(log, 1);
}

void LittleLogs::writeError(String log) {
    writeLog(log, 2);
}

String LittleLogs::readLogs() { return allLogs; }