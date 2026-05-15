// sensor_template.h — Copy this file, rename to {name}_sensor.h, and fill in the blanks.
// Run /add-i2c-sensor in Claude Code to generate this automatically from sensor specs.
//
// Integration checklist (see also .claude/skills/add-i2c-sensor.md):
//   1. Implement the 4 virtual methods below
//   2. Add your sensor to sensors.h / sensors.cpp
//   3. Add PIPO_TYPE string to HW_CONFIG.h
//   4. Add [env:my_variant] block to platformio.ini
//   5. Create configs/{name}_config.json (all 4 sections required)
//   6. Add TypeScript types to web-config/src/types.ts
//   7. Add axis schema to web-config/src/schema.ts

#ifndef PIPO_MYSENSOR_H
#define PIPO_MYSENSOR_H

#include "sensors/input_sensor.h"  // Sensor base class + SensorDat struct
#include "utils/filters.h"         // EMAFilter, MovingAverageFilter, LowPassFilter, KalmanFilter

// Include your sensor's Arduino library here:
// #include <Adafruit_MySensor.h>

// ─── Rename throughout: MySensor → YourSensorName ────────────────────────────

class MySensor : public Sensor {
 public:
  MySensor() {
    // Declare every axis/channel your sensor produces.
    // The key string becomes the OSC address segment and the MIDI channel label.
    // Keep keys short (≤12 chars), snake_case.
    sensor_dat["axis1"] = SensorDat();
    sensor_dat["axis1"].reading_valid = false;

    // Trigger/binary axes (on/off detection):
    sensor_dat["presence"] = SensorDat();
    sensor_dat["presence"].reading_valid = false;
    sensor_dat["presence"].hold_mode = false;   // don't hold last value when signal drops

    // Add more axes as needed:
    // sensor_dat["temp"] = SensorDat();
    // sensor_dat["humidity"] = SensorDat();
  }

  // Called once at boot — set up I2C, initialize library, check sensor is present.
  void init() override;

  // Called after init() — configure measurement parameters (gain, resolution, timing).
  // Also called when config is reloaded. Must be fast (no blocking waits).
  void setup() override;

  // Called at 400Hz by the main loop. Read the hardware; populate raw_value + reading_valid.
  // Return true if the sensor produced output this frame (even if value unchanged).
  // Return false only if the sensor was intentionally skipped (frame-rate limiting).
  //
  // CONTRACT — what you must set per axis:
  //   sensor_dat["key"].raw_value    = hardware reading (float)
  //   sensor_dat["key"].value        = after your optional sensor-specific filter
  //                                    (EMA/MA/LowPass). Skip if no filter needed.
  //   sensor_dat["key"].reading_valid = true if measurement is valid this frame
  //
  // DO NOT call NeutralFilter, apply_offset, or engagement logic — the base class
  // Sensor::update() handles all of that automatically after measure_sensor() returns.
  bool measure_sensor() override;

  // Deserialize sensor-specific runtime config (e.g. gain, mode, threshold).
  // The `inputs` section (deadband, lmin, lmax, etc.) is handled by the base class.
  // Only handle params that live under the "sensorconf" JSON key.
  void set_sensor_config(JsonObject config, bool debug = false) override;

  // Serialize current sensor-specific state back to JSON (mirrors set_sensor_config).
  JsonDocument get_sensor_config(bool debug = false) override;

 private:
  // ── Hardware driver ─────────────────────────────────────────────────────────
  // Adafruit_MySensor _sensor;   // replace with your library's object

  // ── Optional per-axis filters ────────────────────────────────────────────────
  // Choose based on noise level. Use at most one per axis.
  //   EMAFilter:           light, α=0.3–0.7. Good for most continuous sensors.
  //   MovingAverageFilter: heavier, 5–10 samples. Good for noisy range sensors.
  //   LowPassFilter:       frequency cutoff. Good when noise has a known frequency.
  //   KalmanFilter:        expensive, use only if the above are insufficient.
  // EMAFilter _axis1_filter;
  // MovingAverageFilter _axis1_filter;

  // ── Frame-rate limiting (for slow sensors) ───────────────────────────────────
  // If your sensor can't keep up with 400Hz, limit calls with a counter:
  //   uint16_t _frame_counter = 0;
  //   const uint16_t _frame_divisor = 20;  // 400Hz / 20 = 20Hz effective rate
  //
  // In measure_sensor():
  //   if (++_frame_counter < _frame_divisor) return false;
  //   _frame_counter = 0;
  //   // ... proceed with read

  // ── Sensor-specific runtime config fields ────────────────────────────────────
  // Mirror what you read in set_sensor_config():
  // int _gain = 1;
  // bool _high_resolution = false;
};

// Uncomment this extern when you add the global instance in sensors.h/sensors.cpp:
// extern MySensor input_sensor;

#endif  // PIPO_MYSENSOR_H
