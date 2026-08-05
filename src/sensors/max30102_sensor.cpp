#ifdef PIPO_MAX30102

#include "sensors/max30102_sensor.h"
#include "utils/debug.h"

// ---------------------------------------------------------------------------
// Constructor — register all 8 axes in sensor_dat map
// ---------------------------------------------------------------------------
Max30102Sensor::Max30102Sensor() {
  // Raw axes
  sensor_dat["ir_raw"] = SensorDat();
  sensor_dat["red_raw"] = SensorDat();

  // Preconditioned AC axes (pulsatile component)
  sensor_dat["ir_ac"] = SensorDat();
  sensor_dat["red_ac"] = SensorDat();

  // DC baseline axes (tissue + venous absorption)
  sensor_dat["ir_dc"] = SensorDat();
  sensor_dat["red_dc"] = SensorDat();

  // Computed heart rate
  sensor_dat["hr_bpm"] = SensorDat();

  // Die temperature
  sensor_dat["temperature"] = SensorDat();
}

// ---------------------------------------------------------------------------
// init — I2C bus + sensor hardware detection
// ---------------------------------------------------------------------------
void Max30102Sensor::init() {
#if defined(PROTO_ATOM)
  Wire.begin(2, 1, 400000);
#else
  Wire.begin(17, 18, 400000);
#endif

  if (!particleSensor_.begin(Wire, I2C_SPEED_FAST)) {
    log_e("MAX30102 not found on I2C bus");
  } else {
    log_i("MAX30102 found (Part ID: 0x%02X)", particleSensor_.readPartID());
  }
}

// ---------------------------------------------------------------------------
// setup — configure sensor for max-speed acquisition
// ---------------------------------------------------------------------------
void Max30102Sensor::setup() {
  // Max speed config: 1600Hz Red+IR, no averaging, shortest pulse
  // brightness=0x1F(6.4mA), avg=1, mode=2(Red+IR), rate=1600, width=69µs, adc=16384
  particleSensor_.setup(led_brightness_, 1, 2, sample_rate_, pulse_width_,
                        adc_range_);
  particleSensor_.setPulseAmplitudeRed(led_brightness_);
  particleSensor_.setPulseAmplitudeIR(led_brightness_);
  particleSensor_.enableDIETEMPRDY();  // required for readTemperature()
  particleSensor_.clearFIFO();

  // Initialize DC estimator state
  ir_dc_avg_reg_ = 0;
  red_dc_avg_reg_ = 0;

  // Per-axis NeutralFilter: already processed by DC estimator + FIR,
  // so set deadband to 0 (pass-through) for AC/DC/computed axes.
  // Raw axes get a small deadband for noise rejection.
  sensor_dat["ir_raw"].NeutralFilter.setDeadband(0.0);
  sensor_dat["red_raw"].NeutralFilter.setDeadband(0.0);
  sensor_dat["ir_ac"].NeutralFilter.setDeadband(0.0);
  sensor_dat["red_ac"].NeutralFilter.setDeadband(0.0);
  sensor_dat["ir_dc"].NeutralFilter.setDeadband(0.0);
  sensor_dat["red_dc"].NeutralFilter.setDeadband(0.0);
  sensor_dat["hr_bpm"].NeutralFilter.setDeadband(0.0);
  sensor_dat["temperature"].NeutralFilter.setDeadband(0.0);

  log_i("MAX30102 setup: %uHz, avg=1, pulse=%uus, adc=%u", sample_rate_,
        pulse_width_, adc_range_);

  if (DEBUG_HEAP)
    pipoDebugHeap();
}

// ---------------------------------------------------------------------------
// measure_sensor — non-blocking FIFO drain at 400Hz loop rate
//
// Sensor runs at 1600Hz internally (max for Red+IR 2-LED mode).
// Each 400Hz Pipo loop iteration drains ~4 samples from the FIFO.
// ---------------------------------------------------------------------------
bool Max30102Sensor::measure_sensor() {
  bool data_ready = false;

  particleSensor_
      .check();  // non-blocking: drains sensor FIFO → internal buffer

  while (particleSensor_.available()) {
    data_ready = true;

    // --- Read paired Red+IR from the same sample position ---
    uint32_t ir_raw_val = particleSensor_.getFIFOIR();
    uint32_t red_raw_val = particleSensor_.getFIFORed();
    particleSensor_.nextSample();  // advance tail

    // --- Raw axes ---
    sensor_dat["ir_raw"].raw_value = static_cast<float>(ir_raw_val);
    sensor_dat["red_raw"].raw_value = static_cast<float>(red_raw_val);
    sensor_dat["ir_raw"].value = sensor_dat["ir_raw"].raw_value;
    sensor_dat["red_raw"].value = sensor_dat["red_raw"].raw_value;

    // --- DC estimation (EMA, α≈0.0625) ---
    int16_t ir_dc_est =
        averageDCEstimator(&ir_dc_avg_reg_, static_cast<uint16_t>(ir_raw_val));
    int16_t red_dc_est = averageDCEstimator(&red_dc_avg_reg_,
                                            static_cast<uint16_t>(red_raw_val));

    sensor_dat["ir_dc"].value = static_cast<float>(ir_dc_est);
    sensor_dat["red_dc"].value = static_cast<float>(red_dc_est);

    // --- AC extraction (raw - DC → low-pass FIR) ---
    int16_t ir_ac_val =
        lowPassFIRFilter(static_cast<int16_t>(ir_raw_val) - ir_dc_est);
    int16_t red_ac_val =
        lowPassFIRFilter(static_cast<int16_t>(red_raw_val) - red_dc_est);

    sensor_dat["ir_ac"].value = static_cast<float>(ir_ac_val);
    sensor_dat["red_ac"].value = static_cast<float>(red_ac_val);

    // --- Heart rate detection (PBA algorithm on IR AC) ---
    if (checkForBeat(static_cast<int32_t>(ir_ac_val))) {
      unsigned long now = millis();
      if (last_beat_ms_ != 0) {
        unsigned long delta_ms = now - last_beat_ms_;
        if (delta_ms > 0) {
          float instant_bpm = 60000.0f / static_cast<float>(delta_ms);

          // Filter: only accept plausible BPM values (20–255)
          if (instant_bpm >= 20.0f && instant_bpm <= 255.0f) {
            rates_[rate_spot_++] = static_cast<uint8_t>(instant_bpm);
            rate_spot_ %= 4;

            // 4-sample moving average for stability
            uint16_t sum = 0;
            for (uint8_t i = 0; i < 4; i++) {
              sum += rates_[i];
            }
            bpm_ = static_cast<float>(sum) / 4.0f;
          }
        }
      }
      last_beat_ms_ = now;
    }
    sensor_dat["hr_bpm"].value = bpm_;

    // --- Finger detection ---
    // When IR DC is very low, no finger is on the sensor → mark invalid
    bool finger_present = (ir_dc_est > 5000);
    sensor_dat["ir_raw"].reading_valid = finger_present;
    sensor_dat["red_raw"].reading_valid = finger_present;
    sensor_dat["ir_ac"].reading_valid = finger_present;
    sensor_dat["red_ac"].reading_valid = finger_present;
    sensor_dat["ir_dc"].reading_valid = finger_present;
    sensor_dat["red_dc"].reading_valid = finger_present;
    sensor_dat["hr_bpm"].reading_valid = finger_present;
  }

  // --- Temperature read (throttled to every ~5s, blocks ~100ms) ---
  unsigned long now = millis();
  if (now - last_temp_read_ms_ >= kTempReadIntervalMs) {
    sensor_dat["temperature"].value = particleSensor_.readTemperature();
    sensor_dat["temperature"].reading_valid = true;
    last_temp_read_ms_ = now;
  }

  return data_ready;
}

// ---------------------------------------------------------------------------
// set_sensor_config — apply sensorconf section from JSON
// ---------------------------------------------------------------------------
void Max30102Sensor::set_sensor_config(JsonObject config, bool debug) {
  if (debug)
    log_d("Max30102Sensor::set_sensor_config");

  if (config["led_brightness"].is<uint8_t>()) {
    set_led_brightness(config["led_brightness"].as<uint8_t>());
  }

  // Note: sample_rate, pulse_width, adc_range changes require
  // calling setup() again (sensor re-init). These are read from
  // sensorconf on boot via setup(), runtime changes not supported.
  if (config["sample_rate"].is<uint16_t>()) {
    sample_rate_ = config["sample_rate"].as<uint16_t>();
  }
  if (config["pulse_width"].is<uint8_t>()) {
    pulse_width_ = config["pulse_width"].as<uint8_t>();
  }
  if (config["adc_range"].is<uint16_t>()) {
    adc_range_ = config["adc_range"].as<uint16_t>();
  }

  if (debug)
    log_d("Max30102Sensor::set_sensor_config end");
}

// ---------------------------------------------------------------------------
// get_sensor_config — export sensorconf section to JSON
// ---------------------------------------------------------------------------
JsonDocument Max30102Sensor::get_sensor_config(bool debug) {
  JsonDocument config;
  config["led_brightness"] = led_brightness_;
  config["sample_rate"] = sample_rate_;
  config["pulse_width"] = pulse_width_;
  config["adc_range"] = adc_range_;
  return config;
}

// ---------------------------------------------------------------------------
// set_led_brightness — live LED current adjustment
// ---------------------------------------------------------------------------
void Max30102Sensor::set_led_brightness(uint8_t val) {
  led_brightness_ = val;
  particleSensor_.setPulseAmplitudeRed(val);
  particleSensor_.setPulseAmplitudeIR(val);
  log_i("LED brightness set to 0x%02X", val);
}

#endif  // PIPO_MAX30102
