#include "hid/hid_translator.h"

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
  float delta = value - last_value;
  //move if delta is more than 2% of the range
  if (abs(delta) < (input_max - input_min) * 0.02) {
    return 0;
  }

  //Todo: add sensitivity setting
  //should probably scale delta with: pixel/unit
  //temorary solution, % of range
  float delta_scaled = delta * 100.0 / (input_max - input_min);
  int8_t mapped_value = map_linear(
      delta_scaled, -40, 40, -127,
      127);  //mouse is btw -127 and 127 but this would move very fast
  last_value = value;
  return mapped_value;
}

JsonDocument HidTranslator::get_json() const {
  JsonDocument j;
  j["enabled"] = enabled;
  j["addr"] = addr;
  j["addr2"] = addr2;
  j["stroke_mode"] = stroke_mode;
  return j;
}

void HidTranslator::set_from_json(const JsonDocument& j) {
  try {
    enabled = j["enabled"];
    addr = j["addr"].as<string>();
    addr2 = j["addr2"].as<string>();
    stroke_mode = j["stroke_mode"];
  } catch (const std::exception& e) {
    Serial.print("Error: ");
    Serial.println(e.what());
  }
}

bool HidTranslator::is_enabled() {
  return enabled;
}

void HidTranslator::set_enabled(bool value) {
  enabled = value;
}

std::string HidTranslator::get_map_address() {
  return addr;
}

std::string HidTranslator::get_map_address2() {
  return addr2;
}

void HidTranslator::set_map_address(std::string value) {
  addr = value;
}

void HidTranslator::set_map_address2(std::string value) {
  addr2 = value;
}

bool HidTranslator::get_stroke_mode() {
  return stroke_mode;
}

void HidTranslator::set_stroke_mode(bool value) {
  stroke_mode = value;
}
