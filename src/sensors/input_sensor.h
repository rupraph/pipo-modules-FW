#ifndef INPUT_SENSOR_H
#define INPUT_SENSOR_H

#include <Arduino.h>
#include <vector>
#include <unordered_map>
#include <Wire.h>
#include <ArduinoJson.h>
#include "utils/filters.h"
#include "HW_CONFIG.h"

using namespace std;

enum Protocol { OSC, MIDI, HID };

//This is a generic class for sensors.
//It defines the basic structure of a sensor to make it adaptable with the engine and various translators classes

//todo should have a class axis instead of a struct to hold the sensor data

struct SensorDat {

  // Config attributes
  float
      deadband;  // supposed to be % of the total range. value for now. is used to set a deadband around zero + dynamic neutral band filter
  float offset;
  bool inverted;
  float lmax;  // can be used in shcmitt trigger mode for high/low triggers
  float lmin;

  bool mode;     // 0 = continuous, 1 = trigger
  bool th_mode;  // 0 = basic, 1 = window trigger
  bool
      over_out;  // when measured value is above lmax, use min for out when true, or max when false

  bool
      cyclic;  // enables output to be computed on a cyclic range (ie 0-1-0 over range)

  // Live attributes
  float raw_value;  // raw value from sensor (pure reading)
  float value;      // after sensor-specific filtering
  float
      value_prev_measure;  // .value before last measure_sensor() — for hold_mode
  float value_offset;      // after offset applied
  float value_ready;       // final value after neutral filter
  float value_prev;        // previous value_ready for comparison
  bool bool_value;         // boolean output when in trigger mode
  bool bool_value_prev;  // previous value of bool_value

  // Range state: whether the sensor reading is within [lmin, lmax]
  // For sensors that can lose signal (e.g. ToF range), set in_range explicitly
  // in measure_sensor() and mark in_range_set_by_sensor = true.
  // For always-valid sensors (IMU, ADC), the base class auto-computes this
  // from value_ready vs [lmin, lmax] after filtering.

  // -- Sensor validity: did the HW produce a meaningful measurement?
  // Sensors that can lose signal (e.g. ToF) set this to false in measure_sensor().
  // Always-valid sensors (IMU, ADC) leave it at default true.
  bool reading_valid = true;
  bool reading_valid_prev = true;
  uint8_t invalid_count = 0;  // consecutive frames with reading_valid == false

  // -- Bounds check: is value_ready within [lmin, lmax]?
  // Computed by base class for ALL axes after filtering — never set by drivers.
  bool within_bounds = true;
  bool within_bounds_prev = true;

  // -- Composite: debounced(reading_valid) && within_bounds
  // This is what triggers/untriggers and the engine use.
  bool in_range = true;
  bool in_range_prev = true;

  bool hold_mode =
      false;  // when true and !reading_valid, hold previous value instead of updating

  struct trigger_flag {
    bool osc_trig = false;
    bool midi_trig = false;
    bool hid_trig = false;
  };
  // trigger flags used to pass entering/existing the range to the senders which will reset them once used.
  // one for each protocol
  trigger_flag trigger_flags;
  trigger_flag untrigger_flags;
  bool ws_monitor = false;

  StateBasedFilter NeutralFilter;

  // should split in structs for config items and live data.
  SensorDat()
      : deadband(0.0),
        offset(0.0),
        inverted(false),
        raw_value(0.0),
        value(0.0),  // contains the value over the full range in sensor unit.
        value_prev_measure(0.0),
        value_offset(0.0),
        value_ready(0.0),
        value_prev(0.0),
        lmax(1000.0),
        lmin(0.0),
        mode(false),
        th_mode(false),
        bool_value(false),
        bool_value_prev(false),
        reading_valid(true),
        reading_valid_prev(true),
        invalid_count(0),
        within_bounds(true),
        within_bounds_prev(true),
        in_range(true),
        in_range_prev(true),
        NeutralFilter(0) {}
};

class Sensor {
 public:
  virtual void init() = 0;
  virtual void setup() = 0;
  bool update();
  virtual bool measure_sensor() = 0;

  void start_measure_offset(const string& sensor_name);
  void start_measure_offset_all();
  void start_measure_offset_list(const string& channel_list);
  void reset_offset(const string& sensor_name);
  void reset_all_offset();

  void store_previous_values();

  // Check if offset measurement is complete
  bool is_offset_measurement_complete();
  void clear_completion_flag();

  // Get measured offset results for async responses
  void get_measured_offsets(String& output);
  virtual void set_sensor_config(JsonObject config, bool debug = false) = 0;
  virtual JsonDocument get_sensor_config(bool debug = false) = 0;

  // bool test_outside_deadband(const std::string& axis);
  bool is_within_range(const std::string& axis);
  bool is_prev_within_range(const std::string& axis);
  bool process_sensor_triggers();        //return true if any flags were toggled
  bool process_sensor_neutral_filter();  //return true if data changed
  float clip(float value, float min, float max);

  void teleplot_data(string axis);

  // function to measure time between 2 calls
  unsigned long measured_loop_duration();
  unsigned long last_update = 0;
  unsigned long measured_loop = 0;  //ms

  //function to measure time between to points in the code
  void start_duration();
  unsigned long end_duration();
  unsigned long start_time = 0;                  //ms
  unsigned long measured_interval_duration = 0;  //ms

  //config
  void get_inputs_config(JsonDocument& doc, bool debug = false);
  void set_input_config(JsonObject config, bool debug = false);

  //Getter setters
  const unordered_map<string, SensorDat>& get_sensor_dat_map() const;

  bool get_inverted(const std::string& axis);
  void set_inverted(const std::string& axis, bool value);

  float get_deadband(const std::string& axis);
  void set_deadband(const std::string& axis, float value);

  float get_offset(const std::string& axis);
  void set_offset(const std::string& axis, float value);

  float get_value(const std::string& axis);
  float get_value_offset(const std::string& axis);
  float get_value_constrained(const std::string& axis);
  void set_value(const std::string& axis, float value);

  float get_value_prev(const std::string& axis);
  void set_value_prev(const std::string& axis, float value);

  float get_limit_max(const std::string& axis);
  void set_limit_max(const std::string& axis, float value);

  float get_limit_min(const std::string& axis);
  void set_limit_min(const std::string& axis, float value);

  void set_all_trigger(const std::string& axis, bool value);
  void set_all_untrigger(const std::string& axis, bool value);

  void set_trigger_flag(const std::string& axis, Protocol protocol, bool value);
  bool get_trigger_flag(const std::string& axis, Protocol protocol);

  void set_untrigger_flag(const std::string& axis, Protocol protocol,
                          bool value);
  bool get_untrigger_flag(const std::string& axis, Protocol protocol);

  bool get_mode(const std::string& axis);
  void set_mode(const std::string& axis, bool value);

  bool get_threshold_mode(const std::string& axis);
  void set_threshold_mode(const std::string& axis, bool value);

  bool get_bool_value(const std::string& axis);
  void set_bool_value(const std::string& axis, bool value);

  bool get_cyclic(const std::string& axis);
  void set_cyclic(const std::string& axis, bool value);

  bool get_over_out(const std::string& axis);
  void set_over_out(const std::string& axis, bool value);

  void monitor_axis(const std::string& axis);

 private:
  void measure_offset_iter();
  void apply_offset();

  string axis_to_measure_offset;
  bool measure_offset_flag = false;
  bool measure_all = false;            // measure offset for all or for one
  bool measure_list = false;           // measure offset for selected list
  vector<string> channels_to_measure;  // list of channels to measure offset
  int measure_offset_counter = 0;
  float offset = 0;

  // Completion tracking for async responses
  // Initialized to true (no measurement needed by default)
  // Set to false when start_measure_offset*() is called, then back to true when done
  bool offset_measurement_complete = true;

  // Helper function to parse comma-separated channel list
  vector<string> parse_channel_list(const string& channel_list);

 protected:
  unordered_map<string, SensorDat> sensor_dat;
};

#endif  // INPUT_SENSOR_H