// Minimal HW_CONFIG stub for unit tests
#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#define PIPO_TYPE "test"
#define HW_REV 10
#define DEBUG_HEAP false
#define DEBUG_CONFIG false
#define LOG_RECEIVED_OSC false

// Dummy pin definitions
#define WIFI_LED 9
#define BT_LED 10
#define SEND_LED 11
#define LOW_BAT_LED 34
#define MODE_SW 3
#define BAT_VOLTAGE 5
#define PP_SW 8

// Calibration
#define OFFSET_CAL_SAMPLES_NB 50

#endif  // HW_CONFIG_H
