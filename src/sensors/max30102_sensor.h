#ifndef MAX30102_SENSOR_H
#define MAX30102_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "MAX30105.h"
#include "heartRate.h"

class Max30102Sensor : public Sensor {
 public:
  Max30102Sensor();

  void init() override;
  void setup() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

  // Sensor-specific config accessors
  uint8_t get_led_brightness() const { return led_brightness_; }
  void set_led_brightness(uint8_t val);

 private:
  MAX30105 particleSensor_;

  // Sensor configuration (persisted in sensorconf)
  uint8_t led_brightness_ = 0x1F;  // 6.4mA default
  uint16_t sample_rate_ = 1600;    // max for Red+IR 2-LED mode
  uint8_t pulse_width_ = 69;       // shortest pulse (69µs)
  uint16_t adc_range_ = 16384;     // full 18-bit range

  // DC estimator state (separate per channel)
  int32_t ir_dc_avg_reg_ = 0;
  int32_t red_dc_avg_reg_ = 0;

  // Heart rate tracking
  uint8_t rates_[4] = {0};  // ring buffer of last 4 BPM readings
  uint8_t rate_spot_ = 0;
  unsigned long last_beat_ms_ = 0;
  float bpm_ = 0;

  // Temperature read throttling
  unsigned long last_temp_read_ms_ = 0;
  static constexpr unsigned long kTempReadIntervalMs = 5000;
};

#endif  // MAX30102_SENSOR_H
