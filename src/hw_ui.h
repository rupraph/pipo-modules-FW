#ifndef HW_UI_H
#define HW_UI_H

#include <Arduino.h>
#include "HW_CONFIG.h"


class HwUi {
    public:
        HwUi(){
        };

        int PWM_Resolution=8;
        int PWM_FREQ=5000;

        void init();
        void setup();
        void update();
        void set_led(int led, int value);
        void set_mode(int mode);

        
};

#endif //HW_UI_H