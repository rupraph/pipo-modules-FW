#ifndef HID_TRANSLATOR_H
#define HID_TRANSLATOR_H

#include <Arduino.h>
#include "utils/json.hpp"

using namespace std;

// UNDER CONSTRUCTION

class HidTranslator {

 public:
  bool enabled = false;
  string map_address = "a";
  // for keyboard should start with KEY_
  bool stroke_mode = false;
  // stroke mode means key is either pressed once per trigger (false), or maintained (true)
  // int output_max = 255;
  // int output_min = 0;

  HidTranslator();

  int map_linear(float value, float min_input, float max_input, int min_output,
                 int max_output);
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

  bool get_stroke_mode();
  void set_stroke_mode(bool value);

  // int get_output_max();
  // void set_output_max(int value);

  // int get_output_min();
  // void set_output_min(int value);
};

#endif  //HID_TRANSLATOR_H