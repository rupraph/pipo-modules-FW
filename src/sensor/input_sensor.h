#ifndef INPUT_SENSOR_H
#define INPUT_SENSOR_H

#include <Arduino.h>
#include <vector>
#include <unordered_map>
#include <Wire.h>
#include "utils/json.hpp"
#include "utils/filters.h"

using namespace std;
using json = nlohmann::json;

enum Protocol { OSC, MIDI, HID };

//This is a generic class for sensors.
//It defines the basic structure of a sensor to make it adaptable with the engine and various translators classes

struct SensorDat {
  // bool enabled;
  // bool inverted; // should likely move to output section
  float deadzone;  // supposed to be % of the total range. value for now
  float inverted;
  float offset;
  bool invert;
  float value;  // should distinguish raw value from output value and have both
  float value_prev;

  float limit_max;  // can be used in shcmitt trigger mode for high/low triggers
  float limit_min;
  // bool triggered;  // rising edge entering defined range or if using threshold.
  // bool untriggered;  // falling edge leaving defined range.

  bool mode;             // 0 = continuous, 1 = trigger
  bool threshold_mode;   // 0 = basic, 1 = schmitt trigger
  bool bool_value;       // boolean output when in trigger mode
  bool bool_value_prev;  // previous value of bool_value

  struct trigger_flag {
    bool osc_trig = false;
    bool midi_trig = false;
    bool hid_trig = false;
  };

  trigger_flag trigger_flags;
  trigger_flag untrigger_flags;

  // should split in structs for config items and live data.
  SensorDat()
      : deadzone(0.0),
        offset(0.0),
        invert(false),
        value(0.0),
        value_prev(0.0),
        limit_max(1000.0),
        limit_min(0.0),
        mode(false),
        threshold_mode(false),
        bool_value(false),
        bool_value_prev(false) {}
};

class Sensor {
 public:
  virtual void init() = 0;
  virtual void setup() = 0;
  virtual void update() = 0;

  bool test_outside_deadzone(const std::string& axis);
  bool is_within_range(const std::string& axis);
  bool is_prev_within_range(const std::string& axis);
  void process_sensor_triggers();
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
  json get_config(bool debug = false);
  void set_config(json& config, bool debug = false);

  //Getter setters
  unordered_map<string, SensorDat> get_sensor_dat_map();

  bool get_inverted(const std::string& axis);
  void set_inverted(const std::string& axis, bool value);

  int get_deadzone(const std::string& axis);
  void set_deadzone(const std::string& axis, int value);

  float get_offset(const std::string& axis);
  void set_offset(const std::string& axis, float value);

  float get_value(const std::string& axis);
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

 protected:
  unordered_map<string, SensorDat> sensor_dat;
};

#endif  // INPUT_SENSOR_H