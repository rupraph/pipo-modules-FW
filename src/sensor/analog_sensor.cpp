#include "analog_sensor.h"

void AnalogSensor::init() {}

void AnalogSensor::setup() {
  for (auto const& pair : analog_map) {
    pinMode(pair.second, INPUT_PULLDOWN);
  }
  for (auto const& pair : touch_map) {
    pinMode(pair.second, INPUT);
  }

  measure_offset_all();
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

// Todo: add function to perform individual offset or of provided list

void AnalogSensor::measure_offset(const string& sensor_name) {
  int num_samples = OFFSET_CAL_SAMPLES_NB;
  float offset = 0;
  for (int i = 0; i < num_samples; i++) {
    if (analog_map.find(sensor_name) != analog_map.end()) {
      offset += analogRead(analog_map[sensor_name]) * 0.000806;
    } else if (touch_map.find(sensor_name) != touch_map.end()) {
      offset += touchRead(touch_map[sensor_name]);
    }
    delay(20);
  }
  sensor_dat[sensor_name].offset =
      round((offset / num_samples) * 1000.0) / 1000.0;
}

void AnalogSensor::measure_offset_all() {
  // perform intial baseline calibration
  int num_samples = OFFSET_CAL_SAMPLES_NB;
  unordered_map<string, float> offset;
  for (int i = 0; i < num_samples; i++) {

    // measure values without corretcing for offset
    for (auto const& pair : analog_map) {
      sensor_dat[pair.first].value = analogRead(pair.second) * 0.000806;
    }
    for (auto const& pair : touch_map) {
      sensor_dat[pair.first].value = touchRead(pair.second);
    }

    // save measurement
    for (auto const& pair : sensor_dat) {
      offset[pair.first] += sensor_dat[pair.first].value;
    }
    delay(20);
  }
  for (auto const& pair : sensor_dat) {
    sensor_dat[pair.first].offset =
        round((offset[pair.first] / num_samples) * 1000.0) / 1000.0;
  }
}

void AnalogSensor::update() {

  for (auto const& pair : analog_map) {

    sensor_dat[pair.first].raw_value = analogRead(pair.second);
    sensor_dat[pair.first].value_prev = sensor_dat[pair.first].value;
    sensor_dat[pair.first].value = sensor_dat[pair.first].raw_value * 0.000806;
  }

  for (auto const& pair : touch_map) {
    //trial filtering values
    //float val = lp_filter_map[pair.first].process(touchRead(pair.second))-sensor_dat[pair.first].offset;
    sensor_dat[pair.first].raw_value = touchRead(pair.second);

    sensor_dat[pair.first].value_prev = sensor_dat[pair.first].value;

    sensor_dat[pair.first].value =
        sensor_dat[pair.first].raw_value - sensor_dat[pair.first].offset;

    if (sensor_dat[pair.first].value > MAX_TOUCH_VALUE) {
      sensor_dat[pair.first].value = MAX_TOUCH_VALUE;
    }

    //Todo: try to have an adaptative max ? -> when no touch on, min can be adapted.
  }

  process_sensor_triggers();
}
