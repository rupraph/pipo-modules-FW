#ifndef ESP_LOG_STUB_H
#define ESP_LOG_STUB_H

#include <cstdio>

// Stub implementations of Arduino/ESP32 logging macros for unit tests
// These are normally defined in esp32-hal-log.h

#ifdef UNIT_TEST

// Define log macros as printf-style functions for testing
#define log_e(format, ...) printf("[ERROR] " format "\n", ##__VA_ARGS__)
#define log_w(format, ...) printf("[WARN]  " format "\n", ##__VA_ARGS__)
#define log_i(format, ...) printf("[INFO]  " format "\n", ##__VA_ARGS__)
#define log_d(format, ...) printf("[DEBUG] " format "\n", ##__VA_ARGS__)
#define log_v(format, ...) printf("[VERBOSE] " format "\n", ##__VA_ARGS__)

#endif  // UNIT_TEST

#endif  // ESP_LOG_STUB_H
