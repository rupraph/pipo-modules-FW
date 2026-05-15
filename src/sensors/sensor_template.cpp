// sensor_template.cpp — Implementation template. See sensor_template.h for the full guide.
// Rename MySensor throughout to match your class name.

#include "sensors/sensor_template.h"
#include <Arduino.h>

// ── init() ───────────────────────────────────────────────────────────────────
// Called once at boot. Start I2C, verify sensor presence.
// Use log_e() for errors, log_i() for success confirmation.
void MySensor::init() {
  // If your sensor needs a specific I2C bus or pins, start Wire here:
  //   Wire.begin(SDA_PIN, SCL_PIN);
  // Otherwise Wire is already started by the platform before init() is called.

  // Initialize your library:
  //   if (!_sensor.begin(0x44)) {          // 0x44 = I2C address
  //     log_e("MySensor not found");
  //     return;
  //   }

  log_i("MySensor initialized");
}

// ── setup() ──────────────────────────────────────────────────────────────────
// Called after init(), and again whenever config is reloaded.
// Apply measurement settings. No blocking waits.
void MySensor::setup() {
  // Example:
  //   _sensor.setGain(_gain);
  //   _sensor.setResolution(_high_resolution ? HIGH_RES : LOW_RES);

  // Reset filters if any:
  //   _axis1_filter = EMAFilter();
}

// ── measure_sensor() ─────────────────────────────────────────────────────────
// Hot path: runs at 400Hz on Core 1. Keep it fast — no Serial, no heap allocs,
// no blocking waits longer than a single I2C transaction.
bool MySensor::measure_sensor() {
  // ── Optional frame-rate limiter (uncomment for slow sensors) ──────────────
  // if (++_frame_counter < _frame_divisor) return false;
  // _frame_counter = 0;

  // ── Read hardware ─────────────────────────────────────────────────────────
  // bool ok = _sensor.readData();         // typical library call
  // if (!ok) {
  //   sensor_dat["axis1"].reading_valid = false;
  //   return true;  // still return true so base class can apply hold_mode
  // }

  // ── Populate axis1 ────────────────────────────────────────────────────────
  // float raw = _sensor.getAxis1();
  // sensor_dat["axis1"].raw_value = raw;
  //
  // Apply optional sensor-specific filter (skip if no filter needed):
  //   sensor_dat["axis1"].value = _axis1_filter.process(raw);   // EMA
  //   sensor_dat["axis1"].value = _axis1_filter.process(raw, 5); // Moving avg
  // If no filter:
  //   sensor_dat["axis1"].value = raw;
  //
  // sensor_dat["axis1"].reading_valid = true;

  // ── Populate binary/trigger axis ─────────────────────────────────────────
  // float presence_raw = _sensor.getPresence();   // 0.0 or 1.0
  // sensor_dat["presence"].raw_value = presence_raw;
  // sensor_dat["presence"].value = presence_raw;
  // sensor_dat["presence"].reading_valid = true;

  // Return true every frame that produces output (even if values are the same).
  // The base class compares value_ready vs value_prev to detect changes.
  return true;
}

// ── set_sensor_config() ──────────────────────────────────────────────────────
// Reads sensor-specific params from the "sensorconf" JSON section.
// The base class handles "inputs" (deadband, lmin, lmax, etc.) automatically.
void MySensor::set_sensor_config(JsonObject config, bool debug) {
  // Example:
  // if (config["gain"].is<int>())
  //   _gain = config["gain"].as<int>();
  // if (config["high_resolution"].is<bool>())
  //   _high_resolution = config["high_resolution"].as<bool>();

  // After updating fields, re-apply hardware config:
  // setup();
}

// ── get_sensor_config() ──────────────────────────────────────────────────────
// Serializes current sensor-specific state back to JSON (mirrors set_sensor_config).
JsonDocument MySensor::get_sensor_config(bool debug) {
  JsonDocument doc;
  // Example:
  // doc["gain"] = _gain;
  // doc["high_resolution"] = _high_resolution;
  return doc;
}
