#include "hid_translator.h"

using json = nlohmann::json;

// UNDER CONSTRUCTION

HidTranslator::HidTranslator() {}

int HidTranslator::map_linear(float value, float input_min, float input_max,
                              int output_min, int output_max) {
  if (input_min == input_max || output_min == output_max) {
    Serial.println("min and max values cannot be equal");
  }
  // cap value to input range
  if (value < input_min) {
    value = input_min;
  } else if (value > input_max) {
    value = input_max;
  }
  return round((value - input_min) / (input_max - input_min) *
                   (output_max - output_min) +
               output_min);
}

int HidTranslator::get_current_int(float value, float input_min,
                                   float input_max) {
  int mapped_value = map_linear(value, input_min, input_max, 0, 255);
  // if (quantize) {
  //     mapped_value = round(mapped_value / quantize_steps) * quantize_steps;
  // }
  return mapped_value;
}

int8_t HidTranslator::get_mouse_int(float value, float input_min,
                                    float input_max) {
  int8_t mapped_value =
      map_linear(value, input_min, input_max, -32,
                 32);  //mouse is btw -127 and 127 but this would move very fast
  return mapped_value;
}

void to_json(json& j, const HidTranslator& t) {
  j = json{{"map_address", t.map_address},
           {"stroke_mode", t.stroke_mode},
           //  {"output_max", t.output_max},
           //  {"output_min", t.output_min},
           {"enabled", t.enabled}};
}

void from_json(const json& j, HidTranslator& t) {
  j.at("map_address").get_to(t.map_address);
  j.at("stroke_mode").get_to(t.stroke_mode);
  // j.at("output_max").get_to(t.output_max);
  // j.at("output_min").get_to(t.output_min);
  j.at("enabled").get_to(t.enabled);
}

json HidTranslator::get_json() const {
  return json(*this);
}

void HidTranslator::set_from_json(const json& j) {
  *this = j.get<HidTranslator>();
}

std::string HidTranslator::serialize() const {
  json j = *this;
  return j.dump();
}

void HidTranslator::deserialize(const std::string& data) {
  json j = json::parse(data);
  from_json(j, *this);
}

bool HidTranslator::get_enabled() {
  return enabled;
}

void HidTranslator::set_enabled(bool value) {
  enabled = value;
}

std::string HidTranslator::get_map_address() {
  return map_address;
}

void HidTranslator::set_map_address(std::string value) {
  map_address = value;
}

bool HidTranslator::get_stroke_mode() {
  return stroke_mode;
}

void HidTranslator::set_stroke_mode(bool value) {
  stroke_mode = value;
}

// int HidTranslator::get_output_max() {
//   return output_max;
// }

// void HidTranslator::set_output_max(int value) {
//   output_max = value;
// }

// int HidTranslator::get_output_min() {
//   return output_min;
// }

// void HidTranslator::set_output_min(int value) {
//   output_min = value;
// }
