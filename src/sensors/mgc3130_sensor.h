#ifndef PIPO_RANGE_SENSOR_H
#define PIPO_RANGE_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"

#include "DFRobot_MGC3130.h"

#include "utils/filters.h"
#include "utils/debug.h"

class MGC3130Sensor : public Sensor {
 public:
  MGC3130Sensor() {
    sensor_dat["x"] = SensorDat();
    sensor_dat["y"] = SensorDat();
    sensor_dat["z"] = SensorDat();
  };

  void init() override;
  void setup() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

 private:
  DFRobot_MGC3130 mgc3130{14, 12, &Wire};
};

#endif  //MGC3130_SENSOR_H