#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "utils/filters.h"
#include "utils/debug.h"

#ifdef PIPO_ANALOG
// #include "sensors/analog_out.h"
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
#if HW_REV >= 11
    sensor_dat["A07"] = SensorDat();
    sensor_dat["A08"] = SensorDat();
    sensor_dat["T7"] = SensorDat();
    sensor_dat["T8"] = SensorDat();
#endif
  };

#if HW_REV == 10
  unordered_map<string, int> analog_map = {{"A01", 16}, {"A02", 15},
                                           {"A03", 14}, {"A04", 13},
                                           {"A05", 12}, { "A06", 11 }};

  unordered_map<string, int> touch_map = {{"T1", 6}, {"T2", 5}, {"T3", 4},
                                          {"T4", 3}, {"T5", 2}, { "T6", 1 }};
#elif HW_REV >= 11
  unordered_map<string, int> analog_map = {{"A01", 6}, {"A02", 5},  {"A03", 4},
                                           {"A04", 3}, {"A05", 16}, {"A06", 15},
                                           {"A07", 2}, {"A08", 1}};
  unordered_map<string, int> touch_map = {{"T1", 7},  {"T2", 8},  {"T3", 9},
                                          {"T4", 10}, {"T5", 11}, {"T6", 12},
                                          {"T7", 13}, {"T8", 14}};
#endif

  void init() override;
  void setup() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

 private:
  unordered_map<string, MedianFilter> filter_map = {{"A01", MedianFilter(5)},
                                                    {"A02", MedianFilter(5)},
                                                    {"A03", MedianFilter(5)},
                                                    {"A04", MedianFilter(5)},
                                                    {"A05", MedianFilter(5)},
                                                    {"A06", MedianFilter(5)},
                                                    {"T1", MedianFilter(10)},
                                                    {"T2", MedianFilter(10)},
                                                    {"T3", MedianFilter(10)},
                                                    {"T4", MedianFilter(10)},
                                                    {"T5", MedianFilter(10)},
                                                    {"T6", MedianFilter(10)}
#if HW_REV >= 11
                                                    ,
                                                    {"A07", MedianFilter(5)},
                                                    {"A08", MedianFilter(5)},
                                                    {"T7", MedianFilter(10)},
                                                    { "T8",
                                                      MedianFilter(10) }
#endif
  };

  bool touch_adaptative_max;
};

// extern AnalogSensor input_sensor;

#endif  //ANALOG_SENSOR_H