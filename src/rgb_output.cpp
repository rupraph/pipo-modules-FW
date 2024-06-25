#include "rgb_output.h"

RGBOutput rgb_output;

void RGBOutput::setup()
{
    FastLED.setBrightness(RGB_BRIGHNESS);

    set_all_leds(CRGB::White);


    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void RGBOutput::set_led(int led, int r, int g, int b)
{
    leds[led] = CRGB(r, g, b);
}

void RGBOutput::set_led(int led, CRGB color)
{
    leds[led] = color;
}

void RGBOutput::set_all_leds(int r, int g, int b)
{
    for (int i = 0; i < NB_RGB_LEDS; i++)
    {
        leds[i] = CRGB(r, g, b);
    }
}

void RGBOutput::set_all_leds(CRGB color)
{
    for (int i = 0; i < NB_RGB_LEDS; i++)
    {
        leds[i] = color;
    }
}

void RGBOutput::update(Sensor& sensor)
{

    // if (sensor.test_outside_deadzone("x"))
    // {
        
    // }
    // else
    // {
        
    // }
    FastLED.show();

}

