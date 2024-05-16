#include "hw_ui.h"


void HwUi::init()
{

    PWM_Resolution=8;
    PWM_FREQ=5000;

    pinMode(WIFI_LED, OUTPUT);
    pinMode(BT_LED, OUTPUT);
    pinMode(SEND_LED, OUTPUT);
    pinMode(LOW_BAT_LED, OUTPUT);


    pinMode(MODE_SW, INPUT);
    pinMode(BAT_VOLTAGE, INPUT);
    pinMode(PP_SW, INPUT);

}

void HwUi::setup()
{
    //led setup
    ledcSetup(0, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(WIFI_LED, 0);
    ledcSetup(1, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(BT_LED, 1);
    ledcSetup(2, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(SEND_LED, 2);
    ledcSetup(3, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(LOW_BAT_LED, 3);

}

void HwUi::update()
{
}

void HwUi::set_led(int led, int value)
{
    ledcWrite(led, value);
}


     