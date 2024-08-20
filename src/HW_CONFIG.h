#ifndef HW_CONFIG
#define HW_CONFIG

//#define PIPO_RANGE
// could put it as build flag ! like -D PIPO_RANGE

#if defined(PIPO_MOTION)
    #define PIPO_TYPE "motion"
#elif defined(PIPO_RANGE)
    #define PIPO_TYPE "range"
#elif defined(PIPO_ANALOG)
    #define PIPO_TYPE "analog"
#else
    #define PIPO_TYPE "unknown"
#endif

#define PIPO_FW_VERSION "0.1" 


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

// general constants definitions
#define NOTE_BLINK_TIME 20
#define NOTE_BLINK_BRIGHTNESS 80

// other global definitions
#define FORMAT_LITTLEFS_IF_FAILED true


// #if defined(BROADCAST_ADDRESS_DEFINED)
// #else
//     #define BROADCAST_ADDRESS_DEFINED
//     uint8_t broadcastAddress[] = {0x64, 0xE8, 0x33, 0x60, 0x44, 0xF8}; // pipo range address 
// #endif

#endif //HW_CONFIG