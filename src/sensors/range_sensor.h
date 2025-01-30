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
  void update() override;

 private:
  bool within_range = false;
  bool within_range_prev = false;

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