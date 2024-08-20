#ifndef WIFI_TOOLS_H
#define WIFI_TOOLS_H

#include <Arduino.h>
#include "HW_CONFIG.h"
#include "config.h"
#include <WiFiManager.h> 
#include <esp_now.h>
#include "sensor/input_sensor.h"

void setup_wifi();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void esp_now_update(Sensor &sensor);
void setup_esp_now();
void monitor_wifi(bool is_server_runing);
void debug_connect();
extern WiFiManager wm;

// typedef struct struct_message {
//     char a[32];
//     int b;
//     float c;
//     bool d;
//     } struct_message;

#endif //WIFI_TOOLS_H