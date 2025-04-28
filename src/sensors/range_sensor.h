#ifndef RANGE_SENSOR_H
#define RANGE_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include <vl53l4cx_class.h>
#include "utils/filters.h"
#include "utils/debug.h"

class RangeSensor : public Sensor {
 public:
  RangeSensor() { sensor_dat["dist"] = SensorDat(); };

  void init() override;
  void setup() override;
  // void update() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

 private:
  bool within_range = false;
  bool within_range_prev = false;

  float abs_max = 400.0;

  bool hold_mode =
      false;  // define wether to hold the last value or max when no object is detected. true->hold, false->max

  VL53L4CX vl53l4cx;
  VL53L4CX_MultiRangingData_t MultiRangingData;
  VL53L4CX_MultiRangingData_t* pMultiRangingData = &MultiRangingData;
  uint8_t NewDataReady;
  int no_of_object_found;
  char report[64];
  int status;

  SensorDat prev_sensor_dat;

  LowPassFilter lp_filter;
  MovingAverageFilter ma_filter;
  KalmanFilter km_filter;
};

// extern RangeSensor input_sensor;

#endif  //RANGE_SENSOR_H