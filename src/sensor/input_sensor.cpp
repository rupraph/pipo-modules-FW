#include "input_sensor.h"
#include "acc_sensor.h"
#include "range_sensor.h"
// #include "analog_sensor.h"

//Todo: replace throw with Serial

//Todo: deadzone should be in percentage or max or in value ?
// true if outside deadzone
bool Sensor::test_outside_deadzone(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {

    if (abs(sensor_dat[axis].value) > sensor_dat[axis].deadzone) {
      return true;
    } else {
      return false;
    }

  } else {
    // Serial.println("error: Axis not found");
    return false;
  }
}

//provide default implementation
void Sensor::measure_offset(const std::string& sensor_name) {
  Serial.println("default implementation does not measure offset");
}

bool Sensor::is_within_range(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    if (sensor_dat[axis].value > sensor_dat[axis].lmin &&
        sensor_dat[axis].value < sensor_dat[axis].lmax) {
      return true;
    } else {
      return false;
    }
  } else {
    Serial.println("error: Axis not found");
    return false;
  }
}

bool Sensor::is_prev_within_range(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    if (sensor_dat[axis].value_prev > sensor_dat[axis].lmin &&
        sensor_dat[axis].value_prev < sensor_dat[axis].lmax) {
      return true;
    } else {
      return false;
    }
  } else {
    Serial.println("error: Axis not found");
    return false;
  }
}

float Sensor::clip(float value, float min, float max) {
  return std::max(min, std::min(value, max));
}

void Sensor::process_sensor_triggers() {
  for (auto& dat : sensor_dat) {
    string axis = dat.first;
    // warning if reference modifies correctly the value
    SensorDat& axis_data = dat.second;

    axis_data.bool_value_prev = axis_data.bool_value;
    
    // flags for continuous mode
    if (axis_data.mode == 0) {
      if (is_within_range(axis) == false &&
          is_prev_within_range(axis) == true) {
        set_all_untrigger(axis, true);
      }
      if (is_within_range(axis) == true &&
          is_prev_within_range(axis) == false) {
        set_all_trigger(axis, true);
      }
    }

    // flags and bool value for thresh modes
    else if (axis_data.mode == 1) {

      //simple threshold mode
      if (axis_data.th_mode == 0) {
        axis_data.bool_value = axis_data.value > axis_data.lmin;
      } else {
        if (axis_data.th_mode == 1) {
          axis_data.bool_value = axis_data.value > axis_data.lmin &&
                                 axis_data.value < axis_data.lmax;
        }
      }
      // trigger flags for trigger mode
      if (axis_data.bool_value && !axis_data.bool_value_prev) {
        set_all_trigger(axis, true);
      }
      if (!axis_data.bool_value && axis_data.bool_value_prev) {
        set_all_untrigger(axis, true);
      }
    }
  }
}

void Sensor::teleplot_data(string axis) {
  if (sensor_dat.find(axis) == sensor_dat.end()) {
    Serial.println("error: Axis not found");
    return;
  } else {
    Serial.print(">");
    Serial.print(axis.c_str());
    Serial.print(": ");
    Serial.println(sensor_dat[axis].value);
  }
}

unsigned long Sensor::measured_loop_duration() {
  measured_loop = millis() - last_update;
  last_update = millis();
  return measured_loop;
}

void Sensor::start_duration() {
  start_time = millis();
}

unsigned long Sensor::end_duration() {
  measured_interval_duration = millis() - start_time;
  return measured_interval_duration;
}

//config
json Sensor::get_config(bool debug) {
  json config;
  try {
    for (auto const& pair : sensor_dat) {
      string axis_name = pair.first;
      // config[axis_name]["enabled"] = sensor_dat[axis_name].enabled;
      // config[axis_name]["inverted"] = sensor_dat[axis_name].inverted;
      config[axis_name]["deadzone"] = sensor_dat[axis_name].deadzone;
      // config[axis_name]["value"] = sensor_dat[axis_name].value;
      config[axis_name]["offset"] = sensor_dat[axis_name].offset;
      config[axis_name]["lmax"] = sensor_dat[axis_name].lmax;
      config[axis_name]["lmin"] = sensor_dat[axis_name].lmin;
      config[axis_name]["mode"] = sensor_dat[axis_name].mode;
      config[axis_name]["th_mode"] =
          sensor_dat[axis_name].th_mode;
    }
    if (debug) {
      Serial.println("returned_sensor_get_config");
      Serial.println(config.dump(4).c_str());
      Serial.println("returned_sensor_get_config_end");
    }
  } catch (const std::exception& e) {
    Serial.println("error: get_config");
    Serial.println(e.what());
  }

  return config;
}

void Sensor::set_config(json& config, bool debug) {
  for (auto const& pair : config.items()) {
    string axis_name = pair.key();
    sensor_dat[axis_name].deadzone = config[axis_name]["deadzone"];
    sensor_dat[axis_name].offset = config[axis_name]["offset"];
    sensor_dat[axis_name].lmax = config[axis_name]["lmax"];
    sensor_dat[axis_name].lmin = config[axis_name]["lmin"];
    sensor_dat[axis_name].mode = config[axis_name]["mode"];
    sensor_dat[axis_name].th_mode = config[axis_name]["th_mode"];
  }
  if (debug) {
    Serial.println("set_sensor_config");
    Serial.println(config.dump(4).c_str());
    Serial.println("set_sensor_config_end");
  }
}

//getter / setters

unordered_map<string, SensorDat> Sensor::get_sensor_dat_map() {
  return sensor_dat;
}

void Sensor::set_all_trigger(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    sensor_dat[axis].trigger_flags.hid_trig = value;
    sensor_dat[axis].trigger_flags.midi_trig = value;
    sensor_dat[axis].trigger_flags.osc_trig = value;
  } else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_all_untrigger(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    sensor_dat[axis].untrigger_flags.hid_trig = value;
    sensor_dat[axis].untrigger_flags.midi_trig = value;
    sensor_dat[axis].untrigger_flags.osc_trig = value;
  } else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_trigger_flag(const std::string& axis, Protocol protocol,
                              bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    switch (protocol) {
      case Protocol::MIDI:
        sensor_dat[axis].trigger_flags.midi_trig = value;
        break;
      case Protocol::OSC:
        sensor_dat[axis].trigger_flags.osc_trig = value;
        break;
      case Protocol::HID:
        sensor_dat[axis].trigger_flags.hid_trig = value;
        break;
      default:
        throw std::invalid_argument("Protocol not found: " +
                                    std::to_string((int)protocol));
    }
  } else {
    throw std::invalid_argument("Axis not found: " + axis);
  }
}

bool Sensor::get_trigger_flag(const std::string& axis, Protocol protocol) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    switch (protocol) {
      case Protocol::MIDI:
        return sensor_dat[axis].trigger_flags.midi_trig;
      case Protocol::OSC:
        return sensor_dat[axis].trigger_flags.osc_trig;
      case Protocol::HID:
        return sensor_dat[axis].trigger_flags.hid_trig;
      default:
        throw std::invalid_argument("Protocol not found: " +
                                    std::to_string((int)protocol));
    }
  } else {
    throw std::invalid_argument("Axis not found: " + axis);
  }
}

void Sensor::set_untrigger_flag(const std::string& axis, Protocol protocol,
                                bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    switch (protocol) {
      case Protocol::MIDI:
        sensor_dat[axis].untrigger_flags.midi_trig = value;
        break;
      case Protocol::OSC:
        sensor_dat[axis].untrigger_flags.osc_trig = value;
        break;
      case Protocol::HID:
        sensor_dat[axis].untrigger_flags.hid_trig = value;
        break;
      default:
        throw std::invalid_argument("Protocol not found: " +
                                    std::to_string((int)protocol));
    }
  } else {
    throw std::invalid_argument("Axis not found: " + axis);
  }
}

bool Sensor::get_untrigger_flag(const std::string& axis, Protocol protocol) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    switch (protocol) {
      case Protocol::MIDI:
        return sensor_dat[axis].untrigger_flags.midi_trig;
      case Protocol::OSC:
        return sensor_dat[axis].untrigger_flags.osc_trig;
      case Protocol::HID:
        return sensor_dat[axis].untrigger_flags.hid_trig;
      default:
        throw std::invalid_argument("Protocol not found: " +
                                    std::to_string((int)protocol));
    }
  } else {
    throw std::invalid_argument("Axis not found: " + axis);
  }
}

// bool Sensor::get_inverted(const std::string& axis) {
//   if (sensor_dat.find(axis) != sensor_dat.end())
//     return sensor_dat[axis].inverted;
//   else
//     throw std::invalid_argument("Axis not found: " + axis);
// }

int Sensor::get_deadzone(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].deadzone;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_offset(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].offset;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_value(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_value_prev(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].value_prev;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_limit_max(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].lmax;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_limit_min(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].lmin;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

// bool Sensor::get_triggered(const std::string& axis) {
//   if (sensor_dat.find(axis) != sensor_dat.end())
//     return sensor_dat[axis].triggered;
//   else
//     throw std::invalid_argument("Axis not found: " + axis);
// }

// bool Sensor::get_untriggered(const std::string& axis) {
//   if (sensor_dat.find(axis) != sensor_dat.end())
//     return sensor_dat[axis].untriggered;
//   else
//     throw std::invalid_argument("Axis not found: " + axis);
// }

bool Sensor::get_mode(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].mode;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

bool Sensor::get_threshold_mode(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].th_mode;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

bool Sensor::get_bool_value(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].bool_value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

//Setters

// void Sensor::set_enabled(const std::string& axis, bool value) {
//     if(sensor_dat.find(axis) != sensor_dat.end())
//         sensor_dat[axis].enabled = value;
//     else
//         throw std::invalid_argument("Axis not found: " + axis);
// }

// void Sensor::set_inverted(const std::string& axis, bool value) {
//     if(sensor_dat.find(axis) != sensor_dat.end())
//         sensor_dat[axis].inverted = value;
//     else
//         throw std::invalid_argument("Axis not found: " + axis);
// }

void Sensor::set_deadzone(const std::string& axis, int value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].deadzone = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_offset(const std::string& axis, float value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].offset = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_value(const std::string& axis, float value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].value = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_value_prev(const std::string& axis, float value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].value_prev = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_limit_max(const std::string& axis, float value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].lmax = value;

  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_limit_min(const std::string& axis, float value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].lmin = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

// void Sensor::set_triggered(const std::string& axis, bool value) {
//   if (sensor_dat.find(axis) != sensor_dat.end())
//     sensor_dat[axis].triggered = value;
//   else
//     throw std::invalid_argument("Axis not found: " + axis);
// }

// void Sensor::set_untriggered(const std::string& axis, bool value) {
//   if (sensor_dat.find(axis) != sensor_dat.end())
//     sensor_dat[axis].untriggered = value;
//   else
//     throw std::invalid_argument("Axis not found: " + axis);
// }

void Sensor::set_mode(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].mode = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_threshold_mode(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].th_mode = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_bool_value(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].bool_value = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}
