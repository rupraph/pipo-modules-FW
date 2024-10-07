#include "osc_translators.h"

using namespace std;
using json = nlohmann::json;

float OscTranslator::get_value(float value, float input_min, float input_max) {
  if (mode_raw) {
    return value;
  } else {
    return (value - input_min) / (input_max - input_min) *
               (osc_max - osc_min) +
           osc_min;
  }
}

float OscTranslator::get_output_max() {
  return osc_max;
}

float OscTranslator::get_output_min() {
  return osc_min;
}

void OscTranslator::set_output_max(float value) {
  osc_max = value;
}

void OscTranslator::set_output_min(float value) {
  osc_min = value;
}

void to_json(json& j, const OscTranslator& t) {
  j = json{
      {"enabled", t.enabled},
      {"mode_raw", t.mode_raw},
      {"osc_max", t.osc_max},
      {"osc_min", t.osc_min},
  };
}

void from_json(const json& j, OscTranslator& t) {
  j.at("enabled").get_to(t.enabled);
  j.at("mode_raw").get_to(t.mode_raw);
  j.at("osc_max").get_to(t.osc_max);
  j.at("osc_min").get_to(t.osc_min);
}

string OscTranslator::serialize() const {
  json j = *this;
  return j.dump();
}
void OscTranslator::deserialize(const string& data) {
  json j = json::parse(data);
  *this = j.get<OscTranslator>();
}

json OscTranslator::get_json() const {
  return json(*this);
}

void OscTranslator::set_from_json(const json& j) {
  *this = j.get<OscTranslator>();
}
