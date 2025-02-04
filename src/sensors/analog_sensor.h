#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "utils/filters.h"
#include "utils/debug.h"

using namespace std;

class AnalogSensor : public Sensor {
 public:
  AnalogSensor() {
    sensor_dat["A1"] = SensorDat();
    sensor_dat["A2"] = SensorDat();
    sensor_dat["A3"] = SensorDat();
    sensor_dat["A4"] = SensorDat();
    sensor_dat["A5"] = SensorDat();
    sensor_dat["A6"] = SensorDat();
    sensor_dat["T1"] = SensorDat();
    sensor_dat["T2"] = SensorDat();
    sensor_dat["T3"] = SensorDat();
    sensor_dat["T4"] = SensorDat();
    sensor_dat["T5"] = SensorDat();
    sensor_dat["T6"] = SensorDat();
  };

  unordered_map<string, int> analog_map = {{"A1", 16}, {"A2", 15}, {"A3", 14},
                                           {"A4", 13}, {"A5", 12}, {"A6", 11}};

  unordered_map<string, int> touch_map = {{"T1", 6}, {"T2", 5}, {"T3", 4},
                                          {"T4", 3}, {"T5", 2}, {"T6", 1}};

  void init() override;
  void setup() override;
  void update() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;
  void measure_offset(const string& sensor_name) override;
  void measure_offset_all_touch();

 private:
  unordered_map<string, MedianFilter> filter_map = {
      {"A1", MedianFilter(5)},  {"A2", MedianFilter(5)},
      {"A3", MedianFilter(5)},  {"A4", MedianFilter(5)},
      {"A5", MedianFilter(5)},  {"A6", MedianFilter(5)},
      {"T1", MedianFilter(20)}, {"T2", MedianFilter(20)},
      {"T3", MedianFilter(20)}, {"T4", MedianFilter(20)},
      {"T5", MedianFilter(20)}, {"T6", MedianFilter(20)}};

  bool touch_adaptative_max;
};

// extern AnalogSensor input_sensor;

#endif  //ANALOG_SENSOR_H