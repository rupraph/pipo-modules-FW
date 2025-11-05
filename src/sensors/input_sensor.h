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
      deadzone;  // supposed to be % of the total range. value for now. is used to set a deadzone around zero + dynamic neutral band filter
  float offset;
  bool inverted;
  float lmax;  // can be used in shcmitt trigger mode for high/low triggers
  float lmin;

  bool mode;     // 0 = continuous, 1 = trigger
  bool th_mode;  // 0 = basic, 1 = window trigger

  bool
      cyclic;  // enables output to be computed on a cyclic range (ie 0-1-0 over range)

  // Live attributes
  float raw_value;  // raw value from sensor
  float value;  // should distinguish raw value from output value and have both
  float value_prev;
  bool bool_value;       // boolean output when in trigger mode
  bool bool_value_prev;  // previous value of bool_value

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
      : deadzone(0.0),
        offset(0.0),
        inverted(false),
        raw_value(0.0),
        value(0.0),  // contains the value over the full range in sensor unit.
        value_prev(0.0),
        lmax(1000.0),
        lmin(0.0),
        mode(false),
        th_mode(false),
        bool_value(false),
        bool_value_prev(false),
        NeutralFilter(0) {}
};

class Sensor {
 public:
  virtual void init() = 0;
  virtual void setup() = 0;
  void update();
  virtual bool measure_sensor() = 0;
  
  
  void start_measure_offset(const string& sensor_name);
  void start_measure_offset_all();
  void start_measure_offset_list(const string& channel_list);
  void reset_offset(const string& sensor_name);
  void reset_all_offset();
  virtual void set_sensor_config(JsonObject config, bool debug = false) = 0;
  virtual JsonDocument get_sensor_config(bool debug = false) = 0;


  // bool test_outside_deadzone(const std::string& axis);
  bool is_within_range(const std::string& axis);
  bool is_prev_within_range(const std::string& axis);
  void process_sensor_triggers();
  void process_sensor_neutral_filter();
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
  JsonDocument get_inputs_config(bool debug = false);
  void set_input_config(JsonObject config, bool debug = false);

  //Getter setters
  unordered_map<string, SensorDat> get_sensor_dat_map();

  bool get_inverted(const std::string& axis);
  void set_inverted(const std::string& axis, bool value);

  float get_deadzone(const std::string& axis);
  void set_deadzone(const std::string& axis, float value);

  float get_offset(const std::string& axis);
  void set_offset(const std::string& axis, float value);

  float get_value(const std::string& axis);
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

  void monitor_axis(const std::string& axis);

 private:
 void measure_offset_iteration();
void apply_offset();

  string axis_to_measure_offset;
  bool measure_offset_flag = false;
  bool measure_all = false;  // measure offset for all or for one
  bool measure_list = false;  // measure offset for selected list
  vector<string> channels_to_measure;  // list of channels to measure offset
  int measure_offset_counter = 0;
  float offset = 0;
  
  // Helper function to parse comma-separated channel list
  vector<string> parse_channel_list(const string& channel_list);

 protected:
  unordered_map<string, SensorDat> sensor_dat;
};

#endif  // INPUT_SENSOR_H