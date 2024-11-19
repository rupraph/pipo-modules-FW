#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "utils/filters.h"

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
  void measure_offset(const string& sensor_name) override;
  void measure_offset_all();

 private:
  unordered_map<string, LowPassFilter> lp_filter_map = {
      {"A1", LowPassFilter(10)},  {"A2", LowPassFilter(10)},
      {"A3", LowPassFilter(10)},  {"A4", LowPassFilter(10)},
      {"A5", LowPassFilter(10)},  {"A6", LowPassFilter(10)},
      {"T1", LowPassFilter(100)}, {"T2", LowPassFilter(100)},
      {"T3", LowPassFilter(100)}, {"T4", LowPassFilter(100)},
      {"T5", LowPassFilter(100)}, {"T6", LowPassFilter(100)}};

  bool touch_adaptative_max;
};

// extern AnalogSensor input_sensor;

#endif  //ANALOG_SENSOR_H