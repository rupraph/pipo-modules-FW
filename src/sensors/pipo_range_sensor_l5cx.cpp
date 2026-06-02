#ifdef PIPO_RANGE_L5CX

#include "sensors/pipo_range_sensor_l5cx.h"

void PipoRangeSensorL5CX::init() {
  Wire.begin(17, 18, 400000);

  sensor = new VL53L5CX(&Wire, 15);
  sensor->begin();

  int initstatus = sensor->init_sensor(VL53L5CX_DEFAULT_I2C_ADDRESS);
  if (initstatus == 0) {
    log_i("VL53L5CX found and initialized");
  } else {
    log_w("VL53L5CX not found or init failed (status %d)", initstatus);
  }
}

void PipoRangeSensorL5CX::setup() {
  // NB_ZONES drives resolution: 16 = 4x4 mode, 64 = 8x8 mode.
  // Changing NB_ZONES to 64 automatically adapts resolution and zone iteration.
  sensor->vl53l5cx_set_resolution(NB_ZONES);
  sensor->vl53l5cx_set_ranging_frequency_hz(30);  // 30Hz max in 4x4 mode
  sensor->vl53l5cx_start_ranging();

  data_ready = 0;
  ma_filter = MovingAverageFilter(3);

  sensor_dat["dist"].NeutralFilter.setDeadband(0.1);
  sensor_dat["dist"].NeutralFilter.setStableThreshold(0.5);

  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void PipoRangeSensorL5CX::toggle_hold_mode() {
  sensor_dat["dist"].hold_mode = !sensor_dat["dist"].hold_mode;
  log_i("Hold mode %s", sensor_dat["dist"].hold_mode ? "ENABLED" : "DISABLED");
}

bool PipoRangeSensorL5CX::measure_sensor() {
  status = sensor->vl53l5cx_check_data_ready(&data_ready);
  if (status || !data_ready)
    return false;

  status = sensor->vl53l5cx_get_ranging_data(&results);

  // Cache all zone data — ready for future per-zone sensor_dat axes
  float sum = 0.0f;
  int valid_count = 0;
  for (uint8_t z = 0; z < NB_ZONES; z++) {
    // NB_TARGET_PER_ZONE=1 (default): zone z data is at index z
    int idx = VL53L5CX_NB_TARGET_PER_ZONE * z;
    zone_distances[z] = results.distance_mm[idx];
    zone_status[z] = results.target_status[idx];
    zone_nb_targets[z] = results.nb_target_detected[z];

    if (zone_nb_targets[z] > 0 &&
        (zone_status[z] == 5 || zone_status[z] == 9)) {
      sum += zone_distances[z];
      valid_count++;
    }
  }

  bool hw_valid = (valid_count > 0);
  sensor_dat["dist"].reading_valid = hw_valid;
  sensor_dat["presence"].value = sensor_dat["dist"].engaged ? 1.0f : 0.0f;

  if (hw_valid) {
    sensor_dat["dist"].raw_value = (sum / valid_count) / 10.0f;  // mm → cm
    if (!sensor_dat["dist"].reading_valid_prev)
      ma_filter.reset(sensor_dat["dist"].raw_value);
    sensor_dat["dist"].value = ma_filter.process(sensor_dat["dist"].raw_value);
  }

  // Continuous ranging mode restarts automatically — no clear/restart needed

  return true;
}

void PipoRangeSensorL5CX::set_sensor_config(JsonObject config, bool debug) {
  if (debug)
    log_d("set_sensor_config");
  if (config["hold_mode"].is<bool>())
    set_hold_mode(config["hold_mode"].as<bool>());
  if (debug)
    log_d("hold_mode=%d", sensor_dat["dist"].hold_mode);
}

JsonDocument PipoRangeSensorL5CX::get_sensor_config(bool debug) {
  JsonDocument config;
  config["hold_mode"] = sensor_dat["dist"].hold_mode;
  return config;
}

#endif  // PIPO_RANGE_L5CX
