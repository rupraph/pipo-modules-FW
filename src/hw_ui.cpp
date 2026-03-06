#include "hw_ui.h"
#include "utils/config.h"
#include "osc/osc_handler.h"
#include <esp_sleep.h>

HwUi hwui;

// Helper function to safely shutdown the ESP32
void shutdown_esp32() {
  log_w("Battery critically low - shutting down ESP32");

  // Flash LED to indicate shutdown
  for (int i = 0; i < 5; i++) {
    hwui.set_led(LOW_BAT_LED, 100);
#if defined(PIPO_ANALOG) && HW_REV >= 20
    FastLED.show();  // Update WS2812 LEDs
#endif
    delay(100);
    hwui.set_led(LOW_BAT_LED, 0);
#if defined(PIPO_ANALOG) && HW_REV >= 20
    FastLED.show();  // Update WS2812 LEDs
#endif
    delay(100);
  }

  // Turn off all LEDs before shutdown
  hwui.set_led(WIFI_LED, 0);
  hwui.set_led(BT_LED, 0);
  hwui.set_led(SEND_LED, 0);
  hwui.set_led(LOW_BAT_LED, 0);
#if defined(PIPO_ANALOG) && HW_REV >= 20
  FastLED.show();  // Update WS2812 LEDs
#endif

  delay(100);

  // Enter deep sleep (effectively shuts down the ESP32)
  // No wakeup source configured, so it will stay off until power cycled or reset
  esp_deep_sleep_start();
}

void hwuiTask(void* pvParameters) {
  // HWUI task is non-critical - don't subscribe to watchdog
  for (;;) {
    hwui.update();
#if defined(PIPO_ANALOG) && HW_REV >= 20
    vTaskDelay(pdMS_TO_TICKS(20));
#else
    vTaskDelay(pdMS_TO_TICKS(10));
#endif
  }
}

void buttonTask(void* pvParameters) {
  // Button task is non-critical - don't subscribe to watchdog
  for (;;) {
    hwui.update_switches();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void battmonitorTask(void* pvParameters) {
  // Battery monitor is non-critical - don't subscribe to watchdog
  static int prev_bat_percentage = -1;
  static bool prev_plugged_state = false;
  static bool prev_low_battery_state = false;
  static unsigned long last_send_time = 0;
  const unsigned long SEND_INTERVAL = 1000;  // Send every 1 second when changed
  static int shutdown_counter = 0;           // Counter to confirm low voltage
  const int SHUTDOWN_CONFIRM_COUNT =
      10;  // Require 10 consecutive low readings (5 seconds)

  for (;;) {
    // hwui.measure_battery_step();

    // Update the shared state flags
    battery_plugged = hwui.is_plugged();
    battery_low_level = hwui.is_low_battery();

    // Check for critical battery level (shutdown condition)
    // Only shutdown if not plugged in and voltage is critically low
    if (!battery_plugged && hwui.get_bat_voltage() < SHUTDOWN_LEVEL) {
      shutdown_counter++;
      if (shutdown_counter >= SHUTDOWN_CONFIRM_COUNT) {
        shutdown_esp32();  // This will not return
      }
    } else {
      shutdown_counter = 0;  // Reset counter if voltage recovers
    }

    // Check if OSC battery sending is enabled and enough time has passed
    if (config.general_config["OSC_Batt"] == true && osc.is_enabled() &&
        (millis() - last_send_time >= SEND_INTERVAL)) {

      int current_bat_percentage = hwui.get_bat_percentage_int();
      bool current_plugged_state = battery_plugged;
      bool current_low_battery_state = hwui.is_low_battery();

      // Apply hysteresis: only send if change exceeds threshold or plugged state changed
      bool percentage_changed =
          abs(current_bat_percentage - prev_bat_percentage) >=
          BAT_HYSTERESIS_PERCENT;
      bool state_changed = current_plugged_state != prev_plugged_state;
      bool low_battery_changed =
          current_low_battery_state != prev_low_battery_state;

      // Always send on first measurement (prev_bat_percentage == -1)
      if (prev_bat_percentage == -1 || percentage_changed || state_changed ||
          low_battery_changed) {
        // osc.send_battery_level(current_bat_percentage, current_plugged_state,
        //                        current_low_battery_state);
        prev_bat_percentage = current_bat_percentage;
        prev_plugged_state = current_plugged_state;
        prev_low_battery_state = current_low_battery_state;
        last_send_time = millis();
      }
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

//#ifdef PIPO_ANALOG -> for rev1 + output only
// hwuiSoftPwmTask temporarily in the main loop. not smooth when in task
// void hwuiSoftPwmTask(void* pvParameters) {
//   for (;;) {
//     hwui.update_soft_pwm();
//     vTaskDelay(pdMS_TO_TICKS(1) / 10);
//   }
// }
//#endif

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
#if !defined(PIPO_ANALOG)
  // led setup - Arduino 3.x: ledcAttach(pin, freq, resolution)
  ledcAttach(WIFI_LED, PWM_FREQ, PWM_Resolution);
  ledcAttach(BT_LED, PWM_FREQ, PWM_Resolution);
  ledcAttach(SEND_LED, PWM_FREQ, PWM_Resolution);
  ledcAttach(LOW_BAT_LED, PWM_FREQ, PWM_Resolution);
  mode_sw.setup_button(MODE_SW);
#endif
#if defined(PIPO_ANALOG) && HW_REV >= 20
  leds_base_color[WIFI_LED] = CRGB::DarkMagenta;
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

  pause_sw.setup_button(PP_SW);

  log_i("HW UI setup complete");
  hwui.measure_battery();

  //Prevent boot if battery is too low
  if (hwui.get_bat_voltage() < NO_BOOT_VOLTAGE) {
    log_w("Battery too low to boot (%d mV < %d mV)", hwui.get_bat_voltage(),
          NO_BOOT_VOLTAGE);
    // Blink 3 times quickly to warn user
    for (int i = 0; i < 3; i++) {
      hwui.set_led(LOW_BAT_LED, 100);
#if defined(PIPO_ANALOG) && HW_REV >= 20
      FastLED.show();  // Update WS2812 LEDs
#endif
      delay(200);
      hwui.set_led(LOW_BAT_LED, 0);
#if defined(PIPO_ANALOG) && HW_REV >= 20
      FastLED.show();  // Update WS2812 LEDs
#endif
      delay(200);
    }
    delay(100);
    esp_deep_sleep_start();  // Enter deep sleep until power cycled
  }

  start_blink(WIFI_LED, WIFI_AP_PULSE_TIME, 0.2);
  // BT LED blink is started in midiBLESetup() if BLE is enabled

  if (DEBUG_HEAP)
    pipoDebugHeap("End setup hwui");
}

void HwUi::update() {
  monitor_wifiBT_flags();
  blinker();
  pulse();
  single_blink();
  monitor_battery();
#if defined(PIPO_ANALOG) && HW_REV >= 20
  FastLED.show();
#endif
}

void HwUi::monitor_wifiBT_flags() {
  // Read shared flags (volatile) to detect transitions
  bool curSta = staConnected;
  bool curAp = apConnected;
  bool curBT = BTconnected;

  // STA connected -> give steady pulse (slower, to indicate stable connection)
  if (curSta != prev_staConnected) {
    if (curSta) {
      // STA takes priority
      start_pulse(WIFI_LED, WIFI_STA_PULSE_TIME, WIFI_PULSE_MIN_BRIGHTNESS,
                  WIFI_PULSE_BRIGHTNESS);
    } else {
      stop_pulse(WIFI_LED);
      //wait wifi
      start_blink(WIFI_LED, WIFI_AP_PULSE_TIME, 0.2);
      // If STA disconnects but AP is still active, start AP pattern
      if (curAp) {
        start_pulse(WIFI_LED, WIFI_AP_PULSE_TIME, WIFI_PULSE_MIN_BRIGHTNESS,
                    WIFI_PULSE_BRIGHTNESS);
      }
    }
    prev_staConnected = curSta;
  }

  // AP connected -> faster pulse to indicate AP mode (only if STA not connected)
  if (curAp != prev_apConnected) {
    if (curAp && !curSta) {
      start_pulse(WIFI_LED, WIFI_AP_PULSE_TIME, WIFI_PULSE_MIN_BRIGHTNESS,
                  WIFI_PULSE_BRIGHTNESS);
    } else if (!curAp && !curSta) {
      // AP stopped and no STA, turn off LED
      stop_pulse(WIFI_LED);
      //wait wifi
      start_blink(WIFI_LED, WIFI_AP_PULSE_TIME, 0.2);
    }
    // If STA is present, it takes priority (handled in STA logic above)
    prev_apConnected = curAp;
  }

  // BT connected -> steady medium brightness
  if (curBT != prev_BTconnected) {
    log_d("LED: BT state change: %s", curBT ? "CONNECTED" : "DISCONNECTED");
    if (curBT) {
      start_pulse(BT_LED, BT_PULSE_TIME, BT_PULSE_MIN_BRIGHTNESS,
                  BT_PULSE_BRIGHTNESS);
    } else {
      stop_pulse(BT_LED);
      start_blink(BT_LED, WIFI_AP_PULSE_TIME, 0.2);
    }
    prev_BTconnected = curBT;
  }
}

void HwUi::update_switches() {
  // PAUSE has a pullup
  pause_sw.read_debounce();
  if (pause_sw.get_flag()) {
    if (pause_sw.is_short_press()) {
      if (pause_short_press_cb != nullptr) {
        pause_short_press_cb();
      } else {
        // Default behavior if no callback registered
        PAUSED = !PAUSED;
        log_d("Button: PAUSE (short press - default)");
      }
    } else if (pause_sw.is_long_press()) {
      if (pause_long_press_cb != nullptr) {
        pause_long_press_cb();
      } else {
        log_d("Button: PAUSE long press (no handler)");
      }
    }
    pause_sw.reset_button();
  }

#if defined(PIPO_MOTION) || defined(PIPO_RANGE)
  mode_sw.read_debounce();
  if (mode_sw.get_flag()) {
    if (mode_sw.is_short_press()) {
      if (mode_short_press_cb != nullptr) {
        mode_short_press_cb();
      } else {
        log_d("Button: Mode switch short press (no handler)");
      }
    } else if (mode_sw.is_long_press()) {
      if (mode_long_press_cb != nullptr) {
        mode_long_press_cb();
      } else {
        log_d("Button: Mode switch long press (no handler)");
      }
    }
    mode_sw.reset_button();
  }
#endif
}

/**
 * @brief sets brightness of led
 */
void HwUi::set_led(int led_name, int value) {
#if defined(PIPO_MOTION) || defined(PIPO_RANGE)
  ledcWrite(led_name, value);  // Arduino 3.x: ledcWrite takes pin directly
#elif defined(PIPO_ANALOG) && HW_REV == 10
  soft_pwm_table[led_name].brightness = value;
#elif defined(PIPO_ANALOG) && HW_REV == 20
  CRGB color = leds_base_color[led_name];
  color.nscale8_video(value);
  leds[led_name] = color;

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

void HwUi::start_blink(int led_name, int blink_time, float duty_cycle) {
  if (is_blinking(led_name))
    return;  // already blinking

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
  // set_led(led_name, led_blink_table[led_name].brightness);
}

void HwUi::stop_blink(int led_name) {
  if (!is_blinking(led_name))
    return;  // not blinking

  led_blink_table[led_name].enabled = false;
  // set_led(led_name, 0);
}

bool HwUi::is_blinking(int led_name) {
  return led_blink_table[led_name].enabled;
}

void HwUi::start_pulse(int led_name, int pulse_period, int min_brightness,
                       int max_brightness) {
  if (is_pulsing(led_name))
    return;  // already pulsing

  if (is_blinking(led_name)) {
    stop_blink(led_name);
  }
  led_pulse_table[led_name].enabled = true;
  led_pulse_table[led_name].pulse_period = pulse_period;
  led_pulse_table[led_name].min_brightness = min_brightness;
  led_pulse_table[led_name].max_brightness = max_brightness;
  led_pulse_table[led_name].start_cycle = millis();
  // set_led(led_name, led_pulse_table[led_name].min_brightness);
}

void HwUi::stop_pulse(int led_name) {
  if (!is_pulsing(led_name))
    return;  // not pulsing

  led_pulse_table[led_name].enabled = false;
  set_led(led_name, 0);
}

bool HwUi::is_pulsing(int led_name) {
  return led_pulse_table[led_name].enabled;
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

void HwUi::pulse() {
  // this should oscillate the led brightness between min and
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
  blink_once[led_name] = millis() + blink_time;
}

void HwUi::single_blink() {
  for (auto& pair : blink_once) {
    int led_pin = pair.first;
    unsigned long end_time = pair.second;

    if (end_time != 0) {
      if (millis() < end_time) {
        set_led(led_pin, blink_once_brightness);
      } else {
        set_led(led_pin, 0);
        blink_once[led_pin] = 0;
      }
    }
  }
}

// Battery sampling step
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

// Full battery sampling loop
void HwUi::measure_battery() {
  for (int i = 0; i < BAT_SAMPLE_SIZE; i++) {
    measure_battery_step();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// Display battery levels on leds
void HwUi::monitor_battery() {
  if (bat_voltage < LOW_BAT_VOLTAGE) {
    start_blink(LOW_BAT_LED, 500, 0.5);
  } else {
    stop_blink(LOW_BAT_LED);
  }
}

int HwUi::get_bat_voltage() {
  return bat_voltage;
}

float HwUi::get_bat_percentage() {
  // Use same formula as UI: percentage = voltage * 133.3 - 439.8
  // This maps: 3.3V = 0%, 4.05V = 100%
  // bat_voltage is in mV, convert to V first
  float voltage_in_volts = bat_voltage / 1000.0f;

  // Calculate percentage using UI formula
  float percentage = voltage_in_volts * 133.3f - 439.8f;

  // Clamp to 0-100 range
  if (percentage < 0.0f)
    percentage = 0.0f;
  if (percentage > 100.0f)
    percentage = 100.0f;

  return percentage;
}

int HwUi::get_bat_percentage_int() {
  if (battery_plugged) {
    return (int)-1;
  } else {
    return (int)round(get_bat_percentage());
  }
}

bool HwUi::is_plugged() {
  // Device is considered plugged when voltage >= 4.3V
  // bat_voltage is in mV
  return bat_voltage >= 4300;
}

bool HwUi::is_low_battery() {
  // Low battery when voltage is below LOW_BAT_VOLTAGE threshold
  // bat_voltage is in mV
  return bat_voltage < LOW_BAT_VOLTAGE;
}

//Button Class Implementation

void Button::setup_button(int pin) {
  this->pin = pin;
  pinMode(pin, INPUT);
  position = digitalRead(pin);
}

int Button::read_debounce() {
  int current_position = digitalRead(pin);
  unsigned long current_time = millis();

  if (current_position != position) {
    if (current_time - last_press > DEBOUNCE_TIME) {
      position = current_position;
      last_press = current_time;

      if (position == 0) {  // Button pressed (assuming active LOW)
        is_pressed = true;
        press_start_time = current_time;
        long_press_triggered = false;
        last_press_type = PRESS_NONE;
      } else {  // Button released
        is_pressed = false;
        unsigned long press_duration = current_time - press_start_time;

        if (!long_press_triggered) {
          // Only register short press if long press wasn't already triggered
          if (press_duration < LONG_PRESS_TIME) {
            last_press_type = PRESS_SHORT;
          } else {
            last_press_type = PRESS_LONG;
          }
          flag = true;  // button action completed
        }
      }
    }
  }

  // Check for long press while button is held
  if (is_pressed && !long_press_triggered) {
    if (current_time - press_start_time >= LONG_PRESS_TIME) {
      long_press_triggered = true;
      last_press_type = PRESS_LONG;
      flag = true;  // trigger long press immediately
    }
  }

  return position;
}

bool Button::is_long_press() {
  return last_press_type == PRESS_LONG;
}

bool Button::is_short_press() {
  return last_press_type == PRESS_SHORT;
}
