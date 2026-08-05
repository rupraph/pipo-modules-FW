#ifndef MAX30102_SENSOR_H
#define MAX30102_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "MAX30105.h"

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
  // Per-channel PPG signal processing state.
  // Each channel (IR, Red) gets its own independent pipeline.
  struct PPGChannel {
    LowPassFilter dc_filter{0.5f};  // fc≈0.5Hz — tracks baseline drift
    float dc_value = 0.0f;

    LowPassFilter ac_filter{8.0f};  // fc≈8Hz — removes high-freq noise
    float ac_value = 0.0f;

    // Sliding-window envelope via peak/valley detectors with decay.
    // Instant attack, exponential decay: ~90% reduction after 2s at 1600Hz.
    float env_max = 0.0f;
    float env_min = 0.0f;
    static constexpr float kEnvDecay = 0.99928f;

    // Normalized output [0.0, 1.0]: 0=diastole valley, 1=systole peak,
    // 0.5=no confident signal.
    float normalized = 0.5f;

    bool dc_primed = false;  // first-sample init gate

    // Beat detection hysteresis
    bool was_above_systolic = false;

    // Minimum AC amplitude (ADC counts) to consider signal valid.
    static constexpr float kMinAmplitude = 50.0f;
  };

  MAX30105 particleSensor_;

  // Sensor configuration (persisted in sensorconf)
  uint8_t led_brightness_ = 0x1F;  // 6.4mA default
  uint16_t sample_rate_ = 1600;    // max for Red+IR 2-LED mode
  uint8_t pulse_width_ = 69;       // shortest pulse (69µs)
  uint16_t adc_range_ = 16384;     // full 18-bit range

  // Per-channel processing state
  PPGChannel ir_ch_;
  PPGChannel red_ch_;

  // Heart rate tracking (derived from IR normalized signal)
  uint8_t rates_[4] = {0};  // ring buffer of last 4 BPM readings
  uint8_t rate_spot_ = 0;
  unsigned long last_beat_ms_ = 0;
  float bpm_ = 0;

  // Temperature read throttling
  unsigned long last_temp_read_ms_ = 0;
  static constexpr unsigned long kTempReadIntervalMs = 5000;

  // Sample timing (1600Hz internal sensor rate)
  static constexpr float kSampleDeltaT = 1.0f / 1600.0f;

  // Beat detection thresholds on normalized IR signal
  static constexpr float kSystolicThreshold = 0.6f;
  static constexpr float kDiastolicThreshold = 0.4f;

  // Process one PPG channel: raw → DC removal → AC filter → envelope →
  // normalize to [0,1]. Writes .value for dc_axis and ac_axis in sensor_dat.
  void processPPGChannel(PPGChannel& ch, float raw_val, const char* dc_axis,
                         const char* ac_axis);
};

#endif  // MAX30102_SENSOR_H
