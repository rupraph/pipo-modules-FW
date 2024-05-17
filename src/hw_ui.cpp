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

    led_channel_map = {
        {WIFI_LED, 0},
        {BT_LED, 1},
        {SEND_LED, 2},
        {LOW_BAT_LED, 3}
    };

    led_blink_table = {
        {WIFI_LED, {false, 500, 0.5, 0,0, false,30}},
        {BT_LED, {false, 500, 0.5, 0,0, false,30}},
        {SEND_LED, {false, 500, 0.5, 0,0, false,100}},
        {LOW_BAT_LED, {false, 500, 0.5, 0,0, false,100}}
    };

}

void HwUi::setup()
{
    //led setup
    ledcSetup(0, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(WIFI_LED, led_channel_map[WIFI_LED]);
    ledcSetup(1, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(BT_LED, led_channel_map[BT_LED]);
    ledcSetup(2, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(SEND_LED, led_channel_map[SEND_LED]);
    ledcSetup(3, PWM_FREQ, PWM_Resolution);
    ledcAttachPin(LOW_BAT_LED, led_channel_map[LOW_BAT_LED]);

}

void HwUi::update()
{
    blinker();
}

void HwUi::set_led(int led, int value)
{
    ledcWrite(led_channel_map[led], value);
}

void HwUi::start_blink(int led, int blink_time, float duty_cycle)
{
    led_blink_table[led].enabled = true;
    led_blink_table[led].blink_period = blink_time;
    led_blink_table[led].duty_cycle = duty_cycle;
    led_blink_table[led].state = true;
    led_blink_table[led].start_cycle = millis();
    led_blink_table[led].toggle_time = led_blink_table[led].start_cycle + int(led_blink_table[led].duty_cycle*led_blink_table[led].blink_period);
    Serial.println(led_blink_table[led].toggle_time);
    set_led(led, led_blink_table[led].brightness);
}

void HwUi::stop_blink(int led)
{
    led_blink_table[led].enabled = false;
    set_led(led, 0);
    
}

void HwUi::blinker()
{
    unsigned long current_millis = millis();
 
    //loop through led_blink_table
    for (auto& pair : led_blink_table)
    {
        int led_pin = pair.first;
        led_blink& led=led_blink_table[pair.first];

        Serial.println(led.enabled);
        if (led.enabled)
        {
            if (led.state)
            {
                if (current_millis > led.toggle_time)
                {
                    led.state = false;
                    set_led(led_pin, 0);
                }
            }
            else
            {
                if (current_millis > led.start_cycle + led.blink_period)
                {
                    led.state = true;
                    led.start_cycle = current_millis;
                    led.toggle_time = current_millis + int(led.duty_cycle*led.blink_period);
                    set_led(led_pin, led.brightness);
                }
            }
        }
    }
}
     