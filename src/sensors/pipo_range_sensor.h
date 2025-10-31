#ifndef PIPO_RANGE_SENSOR_H
#define PIPO_RANGE_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"

#if HW_REV == 10
#include <vl53l4cx_class.h>
#elif HW_REV >= 11
#include <vl53l1_class.h>
#endif

#include "utils/filters.h"
#include "utils/debug.h"

class PipoRangeSensor : public Sensor {
 public:
  PipoRangeSensor() { sensor_dat["dist"] = SensorDat(); };

  void init() override;
  void setup() override;
  void measure_sensor() override;
  // void measure_offset(const string& sensor_name) override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

 private:
  bool within_range = false;
  bool within_range_prev = false;

  float abs_max = 400.0;

  bool hold_mode =
      false;  // define wether to hold the last value or max when no object is detected. true->hold, false->max

#if HW_REV == 10
  VL53L4CX vl53l4cx;
  VL53L4CX_MultiRangingData_t MultiRangingData;
  VL53L4CX_MultiRangingData_t* pMultiRangingData = &MultiRangingData;
#elif HW_REV >= 11
  VL53L1* vl53l1;
  VL53L1_MultiRangingData_t MultiRangingData;
  VL53L1_MultiRangingData_t* pMultiRangingData = &MultiRangingData;
#endif
  uint8_t NewDataReady;
  int no_of_object_found;
  char report[64];
  int status;

  SensorDat prev_sensor_dat;

  LowPassFilter lp_filter;
  MovingAverageFilter ma_filter;
  KalmanFilter km_filter;
};

// extern PipoRangeSensor input_sensor;

#endif  //RANGE_SENSOR_H