#if defined(PIPO_MOTION)
#define PIPO_TYPE "motion"
#elif defined(PIPO_RANGE)
#define PIPO_TYPE "range"
#elif defined(PIPO_ANALOG)
#define PIPO_TYPE "analog"
#else
#define PIPO_TYPE "unknown"
#endif

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
#define WIFI_LED 8
#define BT_LED 9
#define SEND_LED 10
#define LOW_BAT_LED 34

#define MODE_SW 18
#define BAT_VOLTAGE 17
#define PP_SW 7

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
#define LOW_BAT_VOLTAGE 3300  // in mV
#define BAT_SAMPLE_SIZE 20
#define BATT_COEF 2.0  //2.56
// battery drops hard after 3.3v -> 0%
// max at 4.1.
// perc = volt * 125 -412.5

//analog sensor specific definitions
#define OFFSET_CAL_SAMPLES_NB 50
#define MAX_TOUCH_VALUE 200000

// other global definitions
#define FORMAT_LITTLEFS_IF_FAILED true
