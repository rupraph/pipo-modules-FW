#if defined(PIPO_MOTION)
#define PIPO_TYPE "motion"
#elif defined(PIPO_RANGE)
#define PIPO_TYPE "range"
#define USE_I2C_2V8
#elif defined(PIPO_ANALOG)
#define PIPO_TYPE "analog"
#else
#define PIPO_TYPE "unknown"
#endif

#ifndef HW_REV
#define HW_REV 20  // 1.0 or 1.1
#endif

// #define BETA_OUT

// DEBUG FLAGS
#define DEBUG_HEAP true
#define DEBUG_CONFIG false
#define LOG_RECEIVED_OSC false
#define INCLUDE_BLE

// WebSocket queue size optimization for BLE coexistence
// Reduce queue depth when BLE enabled to prefer fresh data over buffering
// This prevents 1-2 second delays when WiFi STA + BLE are both active
#ifdef INCLUDE_BLE
#define WS_MAX_QUEUED_MESSAGES 8  // Smaller queue = fresher data with BLE
#endif

//HW PINS
#if defined(PIPO_MOTION) || defined(PIPO_RANGE)
#define WIFI_LED 9
#define BT_LED 10
#define SEND_LED 11
#define LOW_BAT_LED 34

#define MODE_SW 3
#define BAT_VOLTAGE 5
#define PP_SW 8

#elif defined(PIPO_ANALOG)
#define BAT_VOLTAGE 17

#if HW_REV == 10
#define WIFI_LED 8
#define BT_LED 9
#define SEND_LED 10
#define LOW_BAT_LED 34
#define MODE_SW 18
#define PP_SW 7
#define OUT_NB 6
#elif HW_REV >= 11
#define PP_SW 21
#define RGB_LED 18
#define OUT_NB 8
#define NB_RGB_LEDS 4
// RGB LED positions, not pins anymore
// #define RGB_BRIGHTNESS 100
#define WIFI_LED 3
#define BT_LED 2
#define SEND_LED 1
#define LOW_BAT_LED 0

#endif

#endif

//LEDS SETTINGS
// general constants definitions
#define NOTE_BLINK_TIME 20
#define NOTE_BLINK_BRIGHTNESS 80
#define WIFI_STA_PULSE_TIME 2000
#define WIFI_AP_PULSE_TIME 1000
#define WIFI_PULSE_BRIGHTNESS 50
#define WIFI_PULSE_MIN_BRIGHTNESS 30
#define BT_PULSE_TIME 3000
#define BT_PULSE_BRIGHTNESS 50
#define BT_PULSE_MIN_BRIGHTNESS 20
// #define ANALOG_TO_VOLTS 0.000806

//ANALOG SETTINGS
#if HW_REV == 10
#define LOW_BAT_VOLTAGE 3400  // in mV //for HW rev 1.0
#define NO_BOOT_VOLTAGE 3100  // in mV (boot if vbat > 3.65)
#define SHUTDOWN_LEVEL 3100   // in mV (seams like it lead vbat 3.35v )
//for HW rev 1.0 (this leads 3.1v at esp under load)   (should have 3v min at esp)
#elif HW_REV >= 11
#define LOW_BAT_VOLTAGE 3400  // in mV //for HW rev 1.1
#define NO_BOOT_VOLTAGE 3150  // in mV
#define CHARGING_LEVEL 4300   // in mV
#define SHUTDOWN_LEVEL 3100   // in mV
#endif

#define BAT_SAMPLE_SIZE 20
#define BAT_HYSTERESIS_PERCENT \
  2  // Minimum percentage change to report (prevents oscillation)
#if HW_REV == 10
#define BATT_COEF 2.0  //2.56
// battery drops hard after 3.3v -> 0% // max at 4.1. // =>percentage = volt * 125 -412.5
#elif HW_REV >= 11
#define BATT_COEF 1.44  //1.436  // divider is 0.7015  // leads 2.95 @4.2
#endif

#define DEBOUNCE_TIME 50      // in ms
#define LONG_PRESS_TIME 1000  // in ms

//analog sensor specific definitions
#define OFFSET_CAL_SAMPLES_NB 50
#define MAX_TOUCH_VALUE 200000

// other global definitions
#define FORMAT_LITTLEFS_IF_FAILED true
