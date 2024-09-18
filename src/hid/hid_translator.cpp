#include "hid_translator.h"

using json = nlohmann::json;

// UNDER CONSTRUCTION

HidTranslator::HidTranslator() {}

int HidTranslator::map_linear(float value, float input_min, float input_max) {
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

int HidTranslator::get_current_bool(float value, float input_min,
                                    float input_max) {
  // if threshold is used, we will return 1 if the value is above the threshold,
  // and 0 if it is below if not, return 1 if value is above the half of the
  // input range, defined by input_max and input_min

  int input_range_center = ((input_max - input_min) + input_min) / 2;

  if (use_threshold) {
    if (value > threshold) {
      return 1;
    } else {
      return 0;
    }
  } else if (value > input_range_center) {
    return 1;
  } else {
    return 0;
  }
}

int HidTranslator::get_current_int(float value, float input_min,
                                   float input_max) {
  int mapped_value = map_linear(value, input_min, input_max);
  // if (quantize) {
  //     mapped_value = round(mapped_value / quantize_steps) * quantize_steps;
  // }
  return mapped_value;
}

void to_json(json& j, const HidTranslator& t) {
  j = json{{"out_type", t.out_type},
           {"mapto", t.mapto},
           {"quantize", t.quantize},
           {"quantize_steps", t.quantize_steps},
           {"use_threshold", t.use_threshold},
           {"threshold", t.threshold},
           {"output_max", t.output_max},
           {"output_min", t.output_min},
           {"enabled", t.enabled}};
}

void from_json(const json& j, HidTranslator& t) {
  j.at("out_type").get_to(t.out_type);
  j.at("mapto").get_to(t.mapto);
  j.at("quantize").get_to(t.quantize);
  j.at("quantize_steps").get_to(t.quantize_steps);
  j.at("use_threshold").get_to(t.use_threshold);
  j.at("threshold").get_to(t.threshold);
  j.at("output_max").get_to(t.output_max);
  j.at("output_min").get_to(t.output_min);
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
