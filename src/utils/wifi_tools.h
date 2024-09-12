#ifndef WIFI_TOOLS_H
#define WIFI_TOOLS_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "HW_CONFIG.h"
#include "config.h"

void setup_wifi();
void monitor_wifi(bool is_server_runing);
void debug_connect();
extern WiFiManager wm;

#endif  //WIFI_TOOLS_H