#ifndef WIFI_TOOLS_H
#define WIFI_TOOLS_H

#include <Arduino.h>
#include "HW_CONFIG.h"
#include "config.h"
#include <WiFiManager.h> 
#include "server_manager.h"

void setup_wifi();
void monitor_wifi(ServerManager& server_manager);

extern WiFiManager wm;

#endif //WIFI_TOOLS_H