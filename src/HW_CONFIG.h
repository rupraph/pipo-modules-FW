#define PIPO_RANGE
// could put it as build flag ! like -D PIPO_RANGE
//#define PROTO_ATOM
#if defined(PIPO_MOTION) || defined(PIPO_RANGE)

    #define WIFI_LED 9
    #define BT_LED 10
    #define SEND_LED 11
    #define LOW_BAT_LED 34 // lowbat is temporarily used as external rgb led data pin

    #define NB_RGB_LEDS 2
    #define RGB_BRIGHNESS 50

    #define MODE_SW 3
    #define BAT_VOLTAGE 5
    #define PP_SW 8

    #define TP_7 4
    #define TP_8 6 
    
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