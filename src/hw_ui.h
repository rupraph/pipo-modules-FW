#ifndef HW_UI_H
#define HW_UI_H

#include <Arduino.h>
#include <unordered_map>
#include "HW_CONFIG.h"

class HwUi {
 public:
  HwUi() {};

  int PWM_Resolution = 8;
  int PWM_FREQ = 5000;
  std::unordered_map<int, int> led_channel_map;

  struct led_blink {
    bool enabled;
    int blink_period;  // in ms
    float duty_cycle;  // 0-1
    unsigned long toggle_time;
    unsigned long start_cycle;
    bool state;
    int brightness;
  };

  struct led_pulse {
    bool enabled;
    int pulse_period;  // in ms
    unsigned long start_cycle;
    int min_brightness;
    int max_brightness;
  };

  static const int NUM_LEDS = 4;

  //Todo: avoid assigning both blink and pulse to same led

  std::unordered_map<int, led_blink>
      led_blink_table;  // position is led_name (ie pin)
  std::unordered_map<int, led_pulse>
      led_pulse_table;  // position is led_name (ie pin)

  unsigned long
      blink_once[NUM_LEDS];  // the position in table are the channel nb.

  void init();
  void setup();
  void update();
  void set_led(int led_name, int value);
  void set_mode(int mode);

  void init_blink_once(int led_name, int blink_time, int brightness);
  void stop_blink_once();

  void start_blink(int led_name, int blink_freq, float duty_cycle);
  void stop_blink(int led_name);

  void blinker();
  void pulse();

  void start_pulse(int led_name, int pulse_period, int min_brightness,
                   int max_brightness);
  void stop_pulse(int led_name);

  bool is_pulsing(int led_name);

  void measure_battery();
  void monitor_battery();

  // getter/setter
  int get_bat_voltage() { return bat_voltage; }

 private:
  int bat_sampling[BAT_SAMPLE_SIZE];
  int bat_sampling_index = 0;
  int bat_voltage = 0;
};

extern HwUi hwui;

#endif  //HW_UI_H