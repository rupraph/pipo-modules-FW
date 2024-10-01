#ifndef OSC_TRANSLATORS_H
#define OSC_TRANSLATORS_H

#include <Arduino.h>
#include "config.h"
#include "utils/json.hpp"

using namespace std;

class OscTranslator {
 public:
  bool enabled = false;
  bool mode_raw = true;  // sends raw sensor data.

  float output_max = 1;
  float output_min = 0;

  float get_value(float value, float min_input, float max_input);

  // Getter/setter
  bool get_enabled();
  bool get_mode_raw();
  float get_output_max();
  float get_output_min();
  void set_enabled(bool value);
  void set_mode_raw(bool value);
  void set_output_max(float value);
  void set_output_min(float value);

  // config
  void to_json(nlohmann::json& j, const OscTranslator& t);
  void from_json(const nlohmann::json& j, OscTranslator& t);

  nlohmann::json get_json() const;
  void set_from_json(const nlohmann::json& j);

  string serialize() const;
  void deserialize(const string& data);

 private:
};

#endif  // OSC_TRANSLATORS_H