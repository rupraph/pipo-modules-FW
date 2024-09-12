#ifndef INPUT_SENSOR_H
#define INPUT_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <unordered_map>
#include <vector>
#include "utils/filters.h"
#include "utils/json.hpp"

using namespace std;
using json = nlohmann::json;

class Sensor {
 public:
  virtual void init() = 0;
  virtual void setup() = 0;
  virtual void update() = 0;

  struct SensorDat {
    bool enabled;
    bool inverted;
    float deadzone;  // supposed to be % of the total range. value for now
    float value;
    float value_prev;
    float offset;
    float limit_max;
    float limit_min;
    bool triggered;  // for now for range, it means entered within range), for
                     // touch, trigger threshold)
    bool untriggered;
    // float threshold;
  };

  void teleplot_data(string axis);
  bool test_outside_deadzone(const std::string& axis);
  bool is_within_range(const std::string& axis);

  // function to measure time between 2 calls
  unsigned long measured_loop_duration();
  unsigned long last_update = 0;
  unsigned long measured_loop = 0;  // ms

  // function to measure time between to points in the code
  void start_duration();
  unsigned long end_duration();
  unsigned long start_time = 0;                  // ms
  unsigned long measured_interval_duration = 0;  // ms

  // config
  json get_config(bool debug = false);
  void set_config(json& config, bool debug = false);

  // Getter setters

  unordered_map<string, SensorDat> get_sensor_dat_map();

  bool get_enabled(const std::string& axis);
  void set_enabled(const std::string& axis, bool value);

  bool get_inverted(const std::string& axis);
  void set_inverted(const std::string& axis, bool value);

  int get_deadzone(const std::string& axis);
  void set_deadzone(const std::string& axis, int value);

  float get_value(const std::string& axis);
  void set_value(const std::string& axis, float value);

  float get_value_prev(const std::string& axis);
  void set_value_prev(const std::string& axis, float value);

  float get_offset(const std::string& axis);
  void set_offset(const std::string& axis, float value);

  float get_limit_max(const std::string& axis);
  void set_limit_max(const std::string& axis, float value);

  float get_limit_min(const std::string& axis);
  void set_limit_min(const std::string& axis, float value);

  float get_triggered(const std::string& axis);
  void set_triggered(const std::string& axis, bool value);

  float get_untriggered(const std::string& axis);
  void set_untriggered(const std::string& axis, bool value);

 protected:
  unordered_map<string, SensorDat> sensor_dat;
};

#endif  // INPUT_SENSOR_H