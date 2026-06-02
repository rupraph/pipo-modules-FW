#ifndef PIPO_RANGE_SENSOR_L5CX_H
#define PIPO_RANGE_SENSOR_L5CX_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include <vl53l5cx_class.h>
#include "utils/filters.h"
#include "utils/debug.h"

class PipoRangeSensorL5CX : public Sensor {
 public:
  PipoRangeSensorL5CX() {
    sensor_dat["dist"] = SensorDat();
    sensor_dat["dist"].reading_valid = false;
    sensor_dat["dist"].reading_valid_prev = false;
    sensor_dat["dist"].hold_mode = false;
    sensor_dat["presence"] = SensorDat();
    sensor_dat["presence"].reading_valid = false;
    sensor_dat["presence"].reading_valid_prev = false;
    sensor_dat["presence"].hold_mode = false;
  }

  void init() override;
  void setup() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

  void toggle_hold_mode();
  bool get_hold_mode() const { return sensor_dat.at("dist").hold_mode; }
  void set_hold_mode(bool mode) { sensor_dat["dist"].hold_mode = mode; }

  // Expose raw zone data for future 4x4 matrix consumers (e.g. plane fitting)
  const int16_t* get_zone_distances() const { return zone_distances; }
  const uint8_t* get_zone_status() const { return zone_status; }
  const uint8_t* get_zone_nb_targets() const { return zone_nb_targets; }
  static constexpr uint8_t NB_ZONES = 16;  // 4x4; set to 64 for 8x8 mode

 private:
  VL53L5CX* sensor;
  VL53L5CX_ResultsData results;

  // Per-zone cache populated every frame — sized for current NB_ZONES.
  // Extend axes in measure_sensor() to expose individual zones as sensor_dat entries.
  int16_t zone_distances[NB_ZONES];  // mm
  uint8_t zone_status[NB_ZONES];     // 5 or 9 = valid
  uint8_t zone_nb_targets[NB_ZONES];

  uint8_t data_ready = 0;
  int status = 0;

  MovingAverageFilter ma_filter;
};

#endif  // PIPO_RANGE_SENSOR_L5CX_H
