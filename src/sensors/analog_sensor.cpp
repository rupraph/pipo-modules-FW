#ifdef PIPO_ANALOG

#include "sensors/analog_sensor.h"
#include <algorithm>

void AnalogSensor::init() {}

void AnalogSensor::setup() {
  for (auto const& pair : analog_map) {
    pinMode(pair.second, INPUT_PULLDOWN);
  }
  for (auto const& pair : touch_map) {
    pinMode(pair.second, INPUT);
  }
#if HW_REV == 10
  start_measure_offset_list("T1,T2,T3,T4,T5,T6");
#elif HW_REV >= 11
  start_measure_offset_list("T1,T2,T3,T4,T5,T6,T7,T8");
#endif
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

bool AnalogSensor::measure_sensor() {
  //WARNING ADC2 pins are not fully usable when wifi is on

  for (auto const& pair : analog_map) {
    if (analog_out.get_pin_dir(pair.first) == PinMode::IN) {
      sensor_dat[pair.first].raw_value = std::min(
          std::max(analogReadMilliVolts(pair.second) / 1000.0f, 0.0f), 3.1f);
      sensor_dat[pair.first].value =
          filter_map[pair.first].process(sensor_dat[pair.first].raw_value);
      ;  // * 0.000806f;
    }
  }

  for (auto const& pair : touch_map) {
    sensor_dat[pair.first].raw_value = touchRead(pair.second);
    sensor_dat[pair.first].value =
        filter_map[pair.first].process(sensor_dat[pair.first].raw_value);

    if (sensor_dat[pair.first].value > MAX_TOUCH_VALUE) {
      sensor_dat[pair.first].value = MAX_TOUCH_VALUE;
    }
  }
  return true;
}

void AnalogSensor::set_sensor_config(JsonObject config, bool debug) {
  if (debug) {
    log_d("set_sensor_config");
  }
  if (config["analogout"].is<JsonObject>()) {
    analog_out.set_config(config["analogout"]);
    log_i("analog_out config set");
  } else {
    log_w("no analog_out config found");
  }
}

JsonDocument AnalogSensor::get_sensor_config(bool debug) {
  JsonDocument config;
  // only analog out to be configured for now
  config["analogout"] = analog_out.get_config();
  return config;
}

#endif