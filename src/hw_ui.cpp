#include "hw_ui.h"

HwUi hwui;

void HwUi::init() {

  PWM_Resolution = 8;
  PWM_FREQ = 5000;

  pinMode(WIFI_LED, OUTPUT);
  pinMode(BT_LED, OUTPUT);
  pinMode(SEND_LED, OUTPUT);
  pinMode(LOW_BAT_LED, OUTPUT);

  // pinMode(MODE_SW, INPUT);
  pinMode(BAT_VOLTAGE, INPUT);
  pinMode(PP_SW, INPUT);

  led_channel_map = {
      {WIFI_LED, 0}, {BT_LED, 1}, {SEND_LED, 2}, {LOW_BAT_LED, 3}};

  led_blink_table = {{WIFI_LED, {false, 500, 0.5, 0, 0, false, 30}},
                     {BT_LED, {false, 500, 0.5, 0, 0, false, 30}},
                     {SEND_LED, {false, 500, 0.5, 0, 0, false, 100}},
                     {LOW_BAT_LED, {false, 500, 0.5, 0, 0, false, 100}}};
  soft_pwm_table = {{WIFI_LED, {true, 0, 255}},
                    {BT_LED, {true, 0, 255}},
                    {SEND_LED, {true, 0, 255}},
                    {LOW_BAT_LED, {true, 0, 255}}};
}

void HwUi::setup() {
#ifndef PIPO_ANALOG&& HW_REV == 10
  // led setup
  ledcSetup(0, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(WIFI_LED, led_channel_map[WIFI_LED]);
  ledcSetup(1, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(BT_LED, led_channel_map[BT_LED]);
  ledcSetup(2, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(SEND_LED, led_channel_map[SEND_LED]);
  ledcSetup(3, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(LOW_BAT_LED, led_channel_map[LOW_BAT_LED]);
#endif
#if defined(PIPO_ANALOG) && HW_REV >= 20
  leds_base_color[WIFI_LED] = CRGB::Yellow;
  leds_base_color[WIFI_LED].nscale8_video(50);
  leds_base_color[BT_LED] = CRGB::SkyBlue;
  leds_base_color[BT_LED].nscale8_video(50);
  leds_base_color[SEND_LED] = CRGB::Red;
  leds_base_color[SEND_LED].nscale8_video(50);
  leds_base_color[LOW_BAT_LED] = CRGB::Orange;
  leds_base_color[LOW_BAT_LED].nscale8_video(50);
  FastLED.addLeds<WS2812, RGB_LED, GRB>(leds, NB_RGB_LEDS);
  // FastLED.setBrightness(0);
  // FastLED.show();
#endif

  set_led(WIFI_LED, 0);
  set_led(BT_LED, 0);
  set_led(SEND_LED, 0);
  set_led(LOW_BAT_LED, 0);
  Serial.println("HW UI setup done");

  if (DEBUG_HEAP)
    pipoDebugHeap("End setup hwui");
}

void HwUi::update() {
  blinker();
  pulse();
  stop_blink_once();
  monitor_battery();
}

/**
 * @brief sets brightness of led
 */
void HwUi::set_led(int led_name, int value) {
#if defined(PIPO_MOTION) || defined(PIPO_RANGE)
  ledcWrite(led_channel_map[led_name], value);
#elif defined(PIPO_ANALOG) && HW_REV == 10
  soft_pwm_table[led_name].brightness = value;
#elif defined(PIPO_ANALOG) && HW_REV == 20
  CRGB color = leds_base_color[led_name];
  color.nscale8_video(value);
  leds[led_name] = color;
  FastLED.show();
#endif
}

/**
 * @brief this writes the soft_pwm to the led pin 
 */
void HwUi::update_soft_pwm() {
  unsigned long current_micros = micros();
  for (auto& pair : soft_pwm_table) {
    int led_pin = pair.first;
    soft_pwm& led = pair.second;
    if (led.enabled) {
      unsigned long toggle_time =
          (led.brightness * soft_pwm_prediod_micros) / 255;

      if (current_micros > (led.start_cycle + soft_pwm_prediod_micros)) {
        led.start_cycle = current_micros;
        continue;
      }
      if (current_micros > (led.start_cycle + toggle_time)) {
        digitalWrite(led_pin, 0);
      } else {
        digitalWrite(led_pin, 1);
      }
    }
  }
}

bool HwUi::is_pulsing(int led_name) {
  return led_pulse_table[led_name].enabled;
}

void HwUi::start_blink(int led_name, int blink_time, float duty_cycle) {
  if (led_pulse_table[led_name].enabled) {
    stop_pulse(led_name);
  }
  led_blink_table[led_name].enabled = true;
  led_blink_table[led_name].blink_period = blink_time;
  led_blink_table[led_name].duty_cycle = duty_cycle;
  led_blink_table[led_name].state = true;
  led_blink_table[led_name].start_cycle = millis();
  led_blink_table[led_name].toggle_time =
      led_blink_table[led_name].start_cycle +
      int(led_blink_table[led_name].duty_cycle *
          led_blink_table[led_name].blink_period);
  set_led(led_name, led_blink_table[led_name].brightness);
}

void HwUi::start_pulse(int led_name, int pulse_period, int min_brightness,
                       int max_brightness) {
  if (led_blink_table[led_name].enabled) {
    stop_blink(led_name);
  }
  led_pulse_table[led_name].enabled = true;
  led_pulse_table[led_name].pulse_period = pulse_period;
  led_pulse_table[led_name].min_brightness = min_brightness;
  led_pulse_table[led_name].max_brightness = max_brightness;
  led_pulse_table[led_name].start_cycle = millis();
  set_led(led_name, led_pulse_table[led_name].min_brightness);
}

void HwUi::stop_blink(int led_name) {
  led_blink_table[led_name].enabled = false;
  set_led(led_name, 0);
}

void HwUi::stop_pulse(int led_name) {
  led_pulse_table[led_name].enabled = false;
  set_led(led_name, 0);
}

void HwUi::blinker() {
  unsigned long current_millis = millis();

  // loop through led_blink_table
  for (auto& pair : led_blink_table) {
    int led_pin = pair.first;
    led_blink& led = led_blink_table[pair.first];

    if (led.enabled) {
      if (led.state) {
        if (current_millis > led.toggle_time) {
          led.state = false;
          set_led(led_pin, 0);
        }
      } else {
        if (current_millis > led.start_cycle + led.blink_period) {
          led.state = true;
          led.start_cycle = current_millis;
          led.toggle_time =
              current_millis + int(led.duty_cycle * led.blink_period);
          set_led(led_pin, led.brightness);
        }
      }
    }
  }
}

void HwUi::
    pulse() {  // this should oscillate the led brightness between min and
               // max brightness
  unsigned long current_millis = millis();
  // loop through led_pulse_table
  for (auto& pair : led_pulse_table) {
    int led_pin = pair.first;
    led_pulse& led = led_pulse_table[pair.first];

    if (led.enabled) {
      int brightness = int(0.5 * (led.max_brightness - led.min_brightness) *
                               sin(2 * PI * (current_millis - led.start_cycle) /
                                   led.pulse_period) +
                           0.5 * (led.max_brightness + led.min_brightness));
      set_led(led_pin, brightness);
    }
  }
}

void HwUi::init_blink_once(int led_name, int blink_time, int brightness) {
  // carfull led_pos used for index in blink_once but comes from channel number
  int led_pos = led_channel_map[led_name];
  set_led(led_name, brightness);
  blink_once[led_pos] = millis() + blink_time;
}

void HwUi::stop_blink_once() {
  for (auto& pair : led_channel_map) {
    int i = pair.second;
    if (millis() > blink_once[i] && blink_once[i] != 0) {
      set_led(pair.first, 0);
      blink_once[i] = 0;
    }
  }
}

void HwUi::measure_battery_step() {
  if (bat_sampling_index >= BAT_SAMPLE_SIZE) {
    bat_sampling_index = 0;
  }
  bat_sampling[bat_sampling_index] =
      analogReadMilliVolts(BAT_VOLTAGE) * BATT_COEF;
  bat_sampling_index++;

  float sum = 0;
  for (int i = 0; i < BAT_SAMPLE_SIZE; i++) {
    sum += bat_sampling[i];
  }
  bat_voltage = sum / BAT_SAMPLE_SIZE;
}

void HwUi::measure_battery() {
  for (int i = 0; i < BAT_SAMPLE_SIZE; i++) {
    measure_battery_step();
    delay(1);
  }
}

void HwUi::monitor_battery() {
  measure_battery_step();
  if (bat_voltage < LOW_BAT_VOLTAGE) {
    start_blink(LOW_BAT_LED, 500, 0.5);
  } else {
    stop_blink(LOW_BAT_LED);
  }
}

int HwUi::get_bat_voltage() {
  return bat_voltage;
}
