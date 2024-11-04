#ifndef OSC_TRANSLATORS_H
#define OSC_TRANSLATORS_H

#include <Arduino.h>
#include "config.h"
#include <ArduinoJson.h>

using namespace std;

class OscTranslator {
 public:
  float get_value(float value, float min_input, float max_input);

  // Getter/setter
  bool get_enabled();
  float get_output_max();
  float get_output_min();
  void set_enabled(bool value);
  void set_mode_raw(bool value);
  void set_output_max(float value);
  void set_output_min(float value);
  void set_osc_addr(string value);
  string get_osc_addr();

  // config
  JsonDocument get_json() const;
  void set_from_json(const JsonDocument& j);

 private:
  bool enabled = false;
  bool mode_raw = true;  // sends raw sensor data.

  float osc_max = 1;
  float osc_min = 0;

  // OSC address
  string osc_addr = "/";
};

#endif  // OSC_TRANSLATORS_H