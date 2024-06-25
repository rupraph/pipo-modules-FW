#ifndef rgb_output_h
#define rgb_output_h

#include <Arduino.h>
#include <FastLED.h>
#include "HW_CONFIG.h"
#include "sensor/input_sensor.h"



class RGBOutput
{
    public:
        RGBOutput()
        {FastLED.addLeds<WS2812, LOW_BAT_LED>(leds,NB_RGB_LEDS);};

        void setup();
        void update(Sensor& sensor);


        void set_led(int led, int r, int g, int b);
        void set_led(int led, CRGB color);
        void set_all_leds(int r, int g, int b);
        void set_all_leds(CRGB color);
        void show();
        void clear();
    private:
        
        CRGB leds[NB_RGB_LEDS];

        CRGBPalette16 currentPalette;
        TBlendType currentBlending;
        
};

extern RGBOutput rgb_output;

#endif //rgb_output_h