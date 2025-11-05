#ifdef PIPO_ANALOG

#include "sensors/analog_sensor.h"
#include <algorithm>

void AnalogSensor::init() {}

void AnalogSensor::setup() {
  for (auto const& pair : analog_map) {
    pinMode(pair.second, INPUT);
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

// Todo: offset measurement to rework for all sensors.
// add function to perform individual offset or of provided list
// change delay into vtaskdelay ??
// void AnalogSensor::measure_offset(const string& sensor_name) {
//   int num_samples = OFFSET_CAL_SAMPLES_NB;
//   float offset = 0;
//   for (int i = 0; i < num_samples; i++) {
//     if (analog_map.find(sensor_name) != analog_map.end() &&
//         analog_out.get_pin_dir(sensor_name) == PinMode::IN) {
//       offset += analogReadMilliVolts(analog_map[sensor_name]) /
//                 1000.0f;  // * 0.000806;
//     } else if (touch_map.find(sensor_name) != touch_map.end()) {
//       offset += touchRead(touch_map[sensor_name]);
//     }
//     vTaskDelay(pdMS_TO_TICKS(2));
//   }
//   sensor_dat[sensor_name].offset =
//       round((offset / num_samples) * 1000.0) / 1000.0;
// }

// measure offset of touch. name is wrong
// void AnalogSensor::measure_offset_all_touch() {
//   // perform intial baseline calibration
//   int num_samples = OFFSET_CAL_SAMPLES_NB;
//   unordered_map<string, float> offset;
//   for (int i = 0; i < num_samples; i++) {

//     // measure values without corretcing for offset
//     // for (auto const& pair : analog_map) {
//     //   sensor_dat[pair.first].value = analogRead(pair.second) * 0.000806;
//     // }
//     for (auto const& pair : touch_map) {
//       sensor_dat[pair.first].value = touchRead(pair.second);
//     }

//     // save measurement
//     for (auto const& pair : sensor_dat) {
//       offset[pair.first] += sensor_dat[pair.first].value;
//     }
//     vTaskDelay(pdMS_TO_TICKS(20));
//   }
//   for (auto const& pair : sensor_dat) {
//     sensor_dat[pair.first].offset =
//         round((offset[pair.first] / num_samples) * 1000.0) / 1000.0;
//   }
// }

// void AnalogSensor::update() {
//   measure_sensor();
//   process_sensor_neutral_filter();
//   process_sensor_triggers();
// }

bool AnalogSensor::measure_sensor() {
  //WARNING ADC2 pins are not fully usable when wifi is on

  for (auto const& pair : analog_map) {
    if (analog_out.get_pin_dir(pair.first) == PinMode::IN) {
      sensor_dat[pair.first].raw_value = std::min(
          std::max(analogReadMilliVolts(pair.second) / 1000.0f, 0.0f), 3.1f);
      sensor_dat[pair.first].value_prev = sensor_dat[pair.first].value;
      sensor_dat[pair.first].value =
          filter_map[pair.first].process(sensor_dat[pair.first].raw_value);
      ;  // * 0.000806f;
    }
  }

  for (auto const& pair : touch_map) {
    //trial filtering values
    //float val = lp_filter_map[pair.first].process(touchRead(pair.second))-sensor_dat[pair.first].offset;
    sensor_dat[pair.first].raw_value = touchRead(pair.second);

    sensor_dat[pair.first].value_prev = sensor_dat[pair.first].value;

    sensor_dat[pair.first].value =
        filter_map[pair.first].process(sensor_dat[pair.first].raw_value);

    if (sensor_dat[pair.first].value > MAX_TOUCH_VALUE) {
      sensor_dat[pair.first].value = MAX_TOUCH_VALUE;
    }

    //Todo: try to have an adaptative max ? -> when no touch on, min can be adapted.
  }
  return true;
}

void AnalogSensor::set_sensor_config(JsonObject config, bool debug) {
  if (debug) {
    Serial.println("set_sensor_config");
  }
  if (config["analogout"].is<JsonObject>()) {
    analog_out.set_config(config["analogout"]);
    Serial.println("analog_out config set");
  } else {
    Serial.println("no analog_out config found");
  }
}

JsonDocument AnalogSensor::get_sensor_config(bool debug) {
  JsonDocument config;
  // only analog out to be configured for now
  config["analogout"] = analog_out.get_config();
  return config;
}

#endif