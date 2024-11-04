#include "osc_translators.h"

using namespace std;

float OscTranslator::get_value(float value, float input_min, float input_max) {
  if (mode_raw) {
    return value;
  } else {
    return (value - input_min) / (input_max - input_min) * (osc_max - osc_min) +
           osc_min;
  }
}

float OscTranslator::get_output_max() {
  return osc_max;
}

float OscTranslator::get_output_min() {
  return osc_min;
}

string OscTranslator::get_osc_addr() {
  return osc_addr;
}

void OscTranslator::set_output_max(float value) {
  osc_max = value;
}

void OscTranslator::set_output_min(float value) {
  osc_min = value;
}

void OscTranslator::set_osc_addr(string value) {
  osc_addr = value;
}

JsonDocument OscTranslator::get_json() const {
  JsonDocument j;
  j["enabled"] = enabled;
  j["mode_raw"] = mode_raw;
  j["osc_max"] = osc_max;
  j["osc_min"] = osc_min;
  j["osc_addr"] = osc_addr;
  return j;
}

void OscTranslator::set_from_json(const JsonDocument& j) {
  try {
    enabled = j["enabled"];
    mode_raw = j["mode_raw"];
    osc_max = j["osc_max"];
    osc_min = j["osc_min"];
    osc_addr = j["osc_addr"].as<string>();
  } catch (const std::exception& e) {
    Serial.print("Error: ");
    Serial.println(e.what());
  }
}

bool OscTranslator::get_enabled() {
  return enabled;
}