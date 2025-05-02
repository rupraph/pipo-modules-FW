#ifndef HW_UI_H
#define HW_UI_H

#include <Arduino.h>
#include <unordered_map>
#include "HW_CONFIG.h"
#include "utils/debug.h"

// This class was initially written to use "ledc" PWM controller to control leds.
// However for Pipo Analog, we use ledc channels to control the analog outputs
// we want 8 pwm outputs possible (while 6 is probaably already nice) and 4 led control, which leads 12 pwm outputs
// This is the reason for the additonal current "soft_pwm" implementation atttempt in this class.
// this is under evluation

// Notes to be re-evaluated when better task distribution implemented:
// - Soft_pwm could not work on core 1 because too busy by the big task at timne of writing and hangs most of the time.
// - somewhat works with occasioanl hanging on core 0 (with prio 1). to be check if serial was interfering
// - seems fine when put in loop (which is anyway running on one of the core with likely a higher priority...)
// to be followed up

class HwUi {
 public:
  HwUi() {};

  int PWM_Resolution = 8;
  int PWM_FREQ = 5000;
  std::unordered_map<int, int> led_channel_map;

  unsigned long soft_pwm_prediod_micros = 5000;

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

  struct soft_pwm {
    bool enabled;
    unsigned long start_cycle;
    int brightness;
  };

  static const int NUM_LEDS = 4;

  //Todo: avoid assigning both blink and pulse to same led

  std::unordered_map<int, led_blink>
      led_blink_table;  // position is led_name (ie pin)
  std::unordered_map<int, led_pulse>
      led_pulse_table;  // position is led_name (ie pin)
  std::unordered_map<int, soft_pwm> soft_pwm_table;

  unsigned long
      blink_once[NUM_LEDS];  // the position in table are the channel nb.

  void init();
  void setup();
  void update();
  void update_soft_pwm();
  void set_led(int led_name, int value);
  void set_mode(int mode);

  void init_blink_once(int led_name, int blink_time, int brightness);
  void stop_blink_once();

  void start_blink(int led_name, int blink_time, float duty_cycle);
  void stop_blink(int led_name);

  void blinker();
  void pulse();

  void start_pulse(int led_name, int pulse_period, int min_brightness,
                   int max_brightness);
  void stop_pulse(int led_name);

  bool is_pulsing(int led_name);

  void measure_battery();
  void measure_battery_step();
  void monitor_battery();

  // getter/setter
  int get_bat_voltage();

 private:
  int bat_sampling[BAT_SAMPLE_SIZE];
  int bat_sampling_index = 0;
  int bat_voltage = 0;
};

extern HwUi hwui;

#endif  //HW_UI_H