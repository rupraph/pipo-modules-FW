#ifndef HID_TRANSLATOR_H
#define HID_TRANSLATOR_H

#include <Arduino.h>
#include "utils/json.hpp"

using namespace std;

// UNDER CONSTRUCTION

class HidTranslator {

 public:
  //Config elements
  bool enabled = false;
  string addr = "a";
  string addr2 = "b";
  // for keyboard should start with KEY_
  bool stroke_mode = false;
  // stroke mode means key is either pressed once per trigger (false), or maintained (true)
  // int output_max = 255;
  // int output_min = 0;

  HidTranslator();

  int map_linear(float value, float min_input, float max_input, int cc_min,
                 int cc_max);
  int get_current_bool(float valu, float min_input, float max_input);
  int get_current_int(float value, float min_input, float max_input);
  int8_t get_mouse_int(float value, float min_input, float max_input);

  // config stuff
  void to_json(nlohmann::json& j, const HidTranslator& t);
  void from_json(const nlohmann::json& j, HidTranslator& t);

  nlohmann::json get_json() const;
  void set_from_json(const nlohmann::json& j);

  string serialize() const;
  void deserialize(const string& data);

  // getter/setters
  bool get_enabled();
  void set_enabled(bool value);

  string get_map_address();
  void set_map_address(string value);

  string get_map_address2();
  void set_map_address2(string value);

  bool get_stroke_mode();
  void set_stroke_mode(bool value);

  // int get_output_max();
  // void set_output_max(int value);

  // int get_output_min();
  // void set_output_min(int value);

 private:
  float last_value = 0;
};

#endif  //HID_TRANSLATOR_H