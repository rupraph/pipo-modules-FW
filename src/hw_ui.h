#ifndef HW_UI_H
#define HW_UI_H

#include <Arduino.h>
#include <unordered_map>
#include <FastLED.h>
#include "HW_CONFIG.h"
#include "utils/debug.h"
#include "shared_flags.h"
#include "esp_task_wdt.h"

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

void hwuiTask(void* pvParameters);
void buttonTask(void* pvParameters);
void battmonitorTask(void* pvParameters);

enum PressType { PRESS_NONE = 0, PRESS_SHORT = 1, PRESS_LONG = 2 };

// Callback type for button press handlers
using ButtonCallback = void (*)();

class Button {
 private:
  int pin;
  int position;
  bool flag = false;  // toogle flag
  unsigned long last_press = 0;
  unsigned long press_start_time = 0;
  bool is_pressed = false;
  bool long_press_triggered = false;
  PressType last_press_type = PRESS_NONE;

 public:
  void setup_button(int pin);
  int read_debounce();
  int get_button();
  void reset_button() {
    flag = false;
    last_press_type = PRESS_NONE;
  }  // reset the button state
  bool get_flag() { return flag; }  // return the button state
  PressType get_press_type() {
    return last_press_type;
  }  // return the type of press detected
  bool is_long_press();   // check if current press is long
  bool is_short_press();  // check if last completed press was short
};
class HwUi {
 public:
  HwUi() {};

  Button pause_sw;
  Button mode_sw;

  int PWM_Resolution = 8;
  int PWM_FREQ = 5000;
  std::unordered_map<int, int> led_channel_map;

  unsigned long soft_pwm_prediod_micros = 5000;

  bool switch_pause;
  unsigned long last_sw_pause_press = 0;

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

  unsigned long blink_once
      [NUM_LEDS];  // register blink start time // the position in table are the channel nb.
  int blink_once_brightness = 100;  // brightness for blink_once led
  void init();
  void setup();
  void update();
  void update_switches();

  void update_soft_pwm();
  void set_led(int led_name, int value);

  void init_blink_once(int led_name, int blink_time, int brightness);
  void single_blink();  //for blink_once led

  void start_blink(int led_name, int blink_time, float duty_cycle);
  void stop_blink(int led_name);
  bool is_blinking(int led_name);

  void start_pulse(int led_name, int pulse_period, int min_brightness,
                   int max_brightness);
  void stop_pulse(int led_name);
  bool is_pulsing(int led_name);

  void blinker();
  void pulse();

  void measure_battery();
  void measure_battery_step();
  void monitor_battery();

  // Monitor shared flags and orchestrate LED patterns
  void monitor_wifiBT_flags();

  // getter/setter
  int get_bat_voltage();

  // Button callback registration
  void set_pause_short_press_callback(ButtonCallback cb) {
    pause_short_press_cb = cb;
  }
  void set_pause_long_press_callback(ButtonCallback cb) {
    pause_long_press_cb = cb;
  }
  void set_mode_short_press_callback(ButtonCallback cb) {
    mode_short_press_cb = cb;
  }
  void set_mode_long_press_callback(ButtonCallback cb) {
    mode_long_press_cb = cb;
  }

 private:
  // Button callbacks
  ButtonCallback pause_short_press_cb = nullptr;
  ButtonCallback pause_long_press_cb = nullptr;
  ButtonCallback mode_short_press_cb = nullptr;
  ButtonCallback mode_long_press_cb = nullptr;
#ifdef PIPO_ANALOG&& HW_REV >= 20
  CRGB leds[NB_RGB_LEDS];
  CRGB leds_base_color[NB_RGB_LEDS];

#endif
  int bat_sampling[BAT_SAMPLE_SIZE];
  int bat_sampling_index = 0;
  int bat_voltage = 0;
  // previous-state tracking for flag transitions
  bool prev_staConnected = false;
  bool prev_apConnected = false;
  bool prev_BTconnected = false;
};

extern HwUi hwui;

#endif  //HW_UI_H