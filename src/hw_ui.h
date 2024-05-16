#ifndef HW_UI_H
#define HW_UI_H

#include <Arduino.h>
#include <unordered_map>
#include "HW_CONFIG.h"


class HwUi {
    public:
        HwUi(){
        };

        int PWM_Resolution=8;
        int PWM_FREQ=5000;
        std::unordered_map<int, int> led_channel_map;

        struct led_blink
        {
            bool enabled;
            int blink_period; // in ms
            float duty_cycle; // 0-1
            unsigned long toggle_time;
            unsigned long start_cycle;
            bool state;
            int brightness;
        };
    
        static const int NUM_LEDS = 4;
        std::unordered_map<int,led_blink> led_blink_table;

        void init();
        void setup();
        void update();
        void set_led(int led, int value);
        void set_mode(int mode);

        void start_blink(int led, int blink_freq, float duty_cycle);
        void stop_blink(int led);
    
        void blinker();

        // led_blink led_blink_table[NUM_LEDS]= {
        // {false, WIFI_LED, 500, 0.5, 0,0, false,255},
        // {false, BT_LED, 500, 0.5, 0,0, false,255},
        // {false, SEND_LED, 500, 0.5, 0,0, false,255},
        // {false, LOW_BAT_LED, 500, 0.5, 0,0, false,255}
        // };

        
};

#endif //HW_UI_H