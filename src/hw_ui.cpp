#include "hw_ui.h"

HwUi hwui;

void HwUi::init() {

  PWM_Resolution = 8;
  PWM_FREQ = 5000;

  pinMode(WIFI_LED, OUTPUT);
  pinMode(BT_LED, OUTPUT);
  pinMode(SEND_LED, OUTPUT);
  pinMode(LOW_BAT_LED, OUTPUT);

  pinMode(MODE_SW, INPUT);
  pinMode(BAT_VOLTAGE, INPUT);
  pinMode(PP_SW, INPUT);

  led_channel_map = {
      {WIFI_LED, 0}, {BT_LED, 1}, {SEND_LED, 2}, {LOW_BAT_LED, 3}};

  led_blink_table = {{WIFI_LED, {false, 500, 0.5, 0, 0, false, 30}},
                     {BT_LED, {false, 500, 0.5, 0, 0, false, 30}},
                     {SEND_LED, {false, 500, 0.5, 0, 0, false, 100}},
                     {LOW_BAT_LED, {false, 500, 0.5, 0, 0, false, 100}}};
}

void HwUi::setup() {
  //led setup
  ledcSetup(0, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(WIFI_LED, led_channel_map[WIFI_LED]);
  ledcSetup(1, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(BT_LED, led_channel_map[BT_LED]);
  ledcSetup(2, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(SEND_LED, led_channel_map[SEND_LED]);
  ledcSetup(3, PWM_FREQ, PWM_Resolution);
  ledcAttachPin(LOW_BAT_LED, led_channel_map[LOW_BAT_LED]);
  set_led(WIFI_LED, 0);
  set_led(BT_LED, 0);
  set_led(SEND_LED, 0);
  set_led(LOW_BAT_LED, 0);
  Serial.println("HW UI setup done");
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

void HwUi::update() {
  blinker();
  pulse();
  stop_blink_once();
  monitor_battery();
}

void HwUi::set_led(int led_name, int value) {
  ledcWrite(led_channel_map[led_name], value);
}

bool HwUi::is_pulsing(int led_name) {
  return led_pulse_table[led_name].enabled;
}

void HwUi::start_blink(int led_name, int blink_time, float duty_cycle) {
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

  //loop through led_blink_table
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
    pulse() {  //this should oscillate the led brightness between min and max brightness
  unsigned long current_millis = millis();
  //loop through led_pulse_table
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

void HwUi::measure_battery() {
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

void HwUi::monitor_battery() {
  measure_battery();
  if (bat_voltage < LOW_BAT_VOLTAGE) {
    start_blink(LOW_BAT_LED, 500, 0.5);
  } else {
    stop_blink(LOW_BAT_LED);
  }
}
