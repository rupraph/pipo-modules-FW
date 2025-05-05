#if defined(PIPO_MOTION)
#define PIPO_TYPE "motion"
#elif defined(PIPO_RANGE)
#define PIPO_TYPE "range"
#elif defined(PIPO_ANALOG)
#define PIPO_TYPE "analog"
#else
#define PIPO_TYPE "unknown"
#endif

#define HW_VERSION 10  // 1.0 or 1.1

// DEBUG FLAGS
#define DEBUG_HEAP true
#define DEBUG_CONFIG false
#define LOG_RECEIVED_OSC false
#define INCLUDE_BLE

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

#if HW_VERSION == 10
#define WIFI_LED 8
#define BT_LED 9
#define SEND_LED 10
#define LOW_BAT_LED 34
#define MODE_SW 18
#define PP_SW 7
#elif HW_VERSION >= 11
#define RGB_LED 18
#define WIFI_LED_POS 4
#define BT_LED_POS 3
#define SEND_LED_POS 2
#define LOW_BAT_LED_POS 1
#define PP_SW 21
#endif

#endif

//LEDS SETTINGS
// general constants definitions
#define NOTE_BLINK_TIME 20
#define NOTE_BLINK_BRIGHTNESS 80
#define WIFI_STA_PULSE_TIME 2000
#define WIFI_AP_PULSE_TIME 1000
#define WIFI_PULSE_BRIGHTNESS 50
// #define ANALOG_TO_VOLTS 0.000806

//ANALOG SETTINGS
#if HW_VERSION == 10
#define LOW_BAT_VOLTAGE 3400  // in mV //for HW rev 1.0
#define NO_BOOT_VOLTAGE 3100  // in mV
//for HW rev 1.0 (this leads 3.1v at esp under load)   (should have 3v min at esp)
#elif HW_VERSION >= 11
#define LOW_BAT_VOLTAGE 3300  // in mV //for HW rev 1.1
#define NO_BOOT_VOLTAGE 3150  // in mV
#define CHARGING_LEVEL 4300   // in mV
#define SHUTDOWN_LEVEL 3100   // in mV
#endif

#define BAT_SAMPLE_SIZE 20
#if HW_VERSION == 10
#define BATT_COEF 2.0  //2.56
// battery drops hard after 3.3v -> 0% // max at 4.1. // perc = volt * 125 -412.5
#elif HW_VERSION >= 11
#define BATT_COEF 0.7015  // leads 2.95 @4.2
#endif

//analog sensor specific definitions
#define OFFSET_CAL_SAMPLES_NB 50
#define MAX_TOUCH_VALUE 200000

// other global definitions
#define FORMAT_LITTLEFS_IF_FAILED true
