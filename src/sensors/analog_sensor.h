#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "utils/filters.h"
#include "utils/debug.h"

#ifdef PIPO_ANALOG
#include "sensors/analog_out.h"
#endif

using namespace std;

class AnalogSensor : public Sensor {
 public:
  AnalogSensor() {
    sensor_dat["A01"] = SensorDat();
    sensor_dat["A02"] = SensorDat();
    sensor_dat["A03"] = SensorDat();
    sensor_dat["A04"] = SensorDat();
    sensor_dat["A05"] = SensorDat();
    sensor_dat["A06"] = SensorDat();
    sensor_dat["T1"] = SensorDat();
    sensor_dat["T2"] = SensorDat();
    sensor_dat["T3"] = SensorDat();
    sensor_dat["T4"] = SensorDat();
    sensor_dat["T5"] = SensorDat();
    sensor_dat["T6"] = SensorDat();
  };

  unordered_map<string, int> analog_map = {{"A01", 16}, {"A02", 15},
                                           {"A03", 14}, {"A04", 13},
                                           {"A05", 12}, {"A06", 11}};

  unordered_map<string, int> touch_map = {{"T1", 6}, {"T2", 5}, {"T3", 4},
                                          {"T4", 3}, {"T5", 2}, {"T6", 1}};

  void init() override;
  void setup() override;
  void update() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;
  void measure_offset(const string& sensor_name) override;
  void measure_offset_all();

 private:
  unordered_map<string, MedianFilter> filter_map = {
      {"A01", MedianFilter(5)}, {"A02", MedianFilter(5)},
      {"A03", MedianFilter(5)}, {"A04", MedianFilter(5)},
      {"A05", MedianFilter(5)}, {"A06", MedianFilter(5)},
      {"T1", MedianFilter(20)}, {"T2", MedianFilter(20)},
      {"T3", MedianFilter(20)}, {"T4", MedianFilter(20)},
      {"T5", MedianFilter(20)}, {"T6", MedianFilter(20)}};

  bool touch_adaptative_max;
};

// extern AnalogSensor input_sensor;

#endif  //ANALOG_SENSOR_H