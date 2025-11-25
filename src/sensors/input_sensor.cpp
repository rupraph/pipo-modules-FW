#include "sensors/input_sensor.h"
// #include "acc_sensor.h"
// #include "range_sensor.h"
// #include "analog_sensor.h"

//Todo: replace throw with Serial

//Todo: deadband should be in percentage or max or in value ?
// true if outside deadband

bool Sensor::update() {
  store_previous_values();
  bool newdata = measure_sensor();

  if (!newdata)
    return false;
  if (measure_offset_flag) {
    measure_offset_iter();
  } else {
    apply_offset();
    process_sensor_neutral_filter();
    process_sensor_triggers();
  }
  return newdata;
}

void Sensor::measure_offset_iter() {
  measure_offset_counter++;
  Serial.print("Offset measurement iteration: ");
  Serial.println(measure_offset_counter);
  if (measure_all) {
    // Measure all channels
    for (auto const& pair : sensor_dat) {
      sensor_dat[pair.first].offset += sensor_dat[pair.first].value;
    }
  } else if (measure_list) {
    // Measure selected list of channels
    for (const string& channel : channels_to_measure) {
      sensor_dat[channel].offset += sensor_dat[channel].value;
    }
  } else {
    // Measure single channel
    sensor_dat[axis_to_measure_offset].offset +=
        sensor_dat[axis_to_measure_offset].value;
  }

  if (measure_offset_counter >= OFFSET_CAL_SAMPLES_NB) {
    // Calculate average offsets
    if (measure_all) {
      for (auto const& pair : sensor_dat) {
        sensor_dat[pair.first].offset =
            sensor_dat[pair.first].offset / OFFSET_CAL_SAMPLES_NB;
      }
      Serial.println("Completed offset measurement for all channels");
    } else if (measure_list) {
      for (const string& channel : channels_to_measure) {
        sensor_dat[channel].offset =
            sensor_dat[channel].offset / OFFSET_CAL_SAMPLES_NB;
      }
      Serial.print("Completed offset measurement for selected channels: ");
      for (const string& channel : channels_to_measure) {
        Serial.print(channel.c_str());
        Serial.print("(");
        Serial.print(sensor_dat[channel].offset);
        Serial.print(") ");
      }
      Serial.println();
    } else {
      sensor_dat[axis_to_measure_offset].offset =
          sensor_dat[axis_to_measure_offset].offset / OFFSET_CAL_SAMPLES_NB;
      Serial.print("offset of ");
      Serial.print(axis_to_measure_offset.c_str());
      Serial.print(" is: ");
      Serial.println(sensor_dat[axis_to_measure_offset].offset);
    }

    // Set completion flag
    offset_measurement_complete = true;

    // Reset measurement flag but keep type flags for get_measured_offsets()
    measure_offset_flag = false;
    // Note: measure_all, measure_list, and channels_to_measure are cleared
    // in clear_completion_flag() after the client retrieves the offsets
  }
}

// bool Sensor::test_outside_deadband(const std::string& axis) {
//   if (sensor_dat.find(axis) != sensor_dat.end()) {

//     if (abs(sensor_dat[axis].value) > sensor_dat[axis].deadband) {
//       return true;
//     } else {
//       return false;
//     }

//   } else {
//     // Serial.println("error: Axis not found");
//     return false;
//   }
// }

//measure single axis offset
void Sensor::start_measure_offset(const std::string& sensor_name) {
  if (!measure_offset_flag) {
    offset_measurement_complete = false;  // Reset completion flag
    measure_offset_flag = true;
    axis_to_measure_offset = sensor_name;
    measure_offset_counter = 0;
    sensor_dat[sensor_name].offset = 0;
    Serial.println("start offset measurement");
  }
}
void Sensor::start_measure_offset_all() {
  if (!measure_offset_flag) {
    offset_measurement_complete = false;  // Reset completion flag
    measure_offset_flag = true;
    measure_all = true;
    measure_offset_counter = 0;
  }
  for (auto const& pair : sensor_dat) {
    sensor_dat[pair.first].offset = 0;
  }
  Serial.println("start all offset measurement");
}

void Sensor::start_measure_offset_list(const string& channel_list) {
  if (!measure_offset_flag) {
    channels_to_measure = parse_channel_list(channel_list);

    // Validate all channels exist
    for (const string& channel : channels_to_measure) {
      if (sensor_dat.find(channel) == sensor_dat.end()) {
        Serial.print("Error: Channel not found: ");
        Serial.println(channel.c_str());
        return;
      }
    }

    // Reset offsets for selected channels
    for (const string& channel : channels_to_measure) {
      sensor_dat[channel].offset = 0;
    }

    offset_measurement_complete = false;  // Reset completion flag
    measure_list = true;
    measure_offset_flag = true;
    measure_offset_counter = 0;

    Serial.print("Starting offset measurement for channels: ");
    Serial.println(channel_list.c_str());
  }
}

vector<string> Sensor::parse_channel_list(const string& channel_list) {
  vector<string> channels;
  string current_channel;

  for (char c : channel_list) {
    if (c == ',' || c == ' ') {
      if (!current_channel.empty()) {
        channels.push_back(current_channel);
        current_channel.clear();
      }
    } else {
      current_channel += c;
    }
  }

  // Add the last channel if not empty
  if (!current_channel.empty()) {
    channels.push_back(current_channel);
  }

  return channels;
}

void Sensor::apply_offset() {
  for (auto const& pair : sensor_dat) {
    sensor_dat[pair.first].value_ready =
        sensor_dat[pair.first].value - sensor_dat[pair.first].offset;
  }
}

void Sensor::reset_offset(const std::string& sensor_name) {
  if (sensor_dat.find(sensor_name) != sensor_dat.end()) {
    Serial.print("reset offset of ");
    Serial.println(sensor_name.c_str());
    sensor_dat[sensor_name].offset = 0;
  } else {
    Serial.println("error: Axis not found");
  }
}

void Sensor::reset_all_offset() {
  for (auto const& pair : sensor_dat) {
    sensor_dat[pair.first].offset = 0;
  }
  Serial.println("reset all offset");
}

void Sensor::store_previous_values() {
  for (auto& dat : sensor_dat) {
    dat.second.value_prev = dat.second.value_ready;
  }
}

// void Sensor::offset_handler() {

//   if (!measure_offset_flag) {
//     //
//   }

//   if (measure_offset_flag) {
//     measure_offset_counter++;
//     offset += sensor_dat[axis_to_measure_offset].value;
//     if (measure_offset_counter >= OFFSET_CAL_SAMPLES_NB) {
//       sensor_dat[axis_to_measure_offset].offset =
//           offset / OFFSET_CAL_SAMPLES_NB;
//       measure_offset_flag = false;
//       Serial.print("offset of ");
//       Serial.print(axis_to_measure_offset.c_str());
//       Serial.print(" is: ");
//       Serial.println(sensor_dat[axis_to_measure_offset].offset);
//     }
//   }
// }

// void Sensor::reset_offset(const std::string& input) {
//   if (sensor_dat.find(input) != sensor_dat.end()) {
//     sensor_dat[input].offset = 0;
//     Serial.print("reset offset for ");
//     Serial.println(input.c_str());
//   } else {
//     Serial.println("error: Axis not found");
//   }
// }

bool Sensor::is_within_range(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    if (sensor_dat[axis].value_ready > sensor_dat[axis].lmin &&
        sensor_dat[axis].value_ready < sensor_dat[axis].lmax) {
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
        axis_data.bool_value = axis_data.value_ready > axis_data.lmin;
      } else {
        if (axis_data.th_mode == 1) {
          axis_data.bool_value = axis_data.value_ready > axis_data.lmin &&
                                 axis_data.value_ready < axis_data.lmax;
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
/**
 * @brief This applies the dynamic dead band filter to the sensor data
 * returns true if data changed after filtering
 */
bool Sensor::process_sensor_neutral_filter() {
  bool data_changed = false;
  for (auto& dat : sensor_dat) {
    string axis = dat.first;
    SensorDat& axis_data = dat.second;
    float new_value = axis_data.NeutralFilter.process(axis_data.value_ready);
    if (new_value != axis_data.value_prev) {
      data_changed = true;
      axis_data.value_ready = new_value;
    }
  }
  return data_changed;
}

void Sensor::teleplot_data(string axis) {
  if (sensor_dat.find(axis) == sensor_dat.end()) {
    Serial.println("error: Axis not found");
    return;
  } else {
    Serial.print(">");
    Serial.print(axis.c_str());
    Serial.print("raw_value: ");
    Serial.println(sensor_dat[axis].raw_value);
    Serial.print(">");
    Serial.print(axis.c_str());
    Serial.print("value ready: ");
    Serial.println(sensor_dat[axis].value_ready);
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
JsonDocument Sensor::get_inputs_config(bool debug) {
  JsonDocument config;
  try {
    for (auto const& pair : sensor_dat) {
      string axis_name = pair.first;
      // config[axis_name]["enabled"] = sensor_dat[axis_name].enabled;
      config[axis_name]["inverted"] = sensor_dat[axis_name].inverted;
      config[axis_name]["deadband"] = sensor_dat[axis_name].deadband;
      // config[axis_name]["value"] = sensor_dat[axis_name].value;
      config[axis_name]["offset"] = sensor_dat[axis_name].offset;
      config[axis_name]["lmax"] = sensor_dat[axis_name].lmax;
      config[axis_name]["lmin"] = sensor_dat[axis_name].lmin;
      config[axis_name]["mode"] = sensor_dat[axis_name].mode;
      config[axis_name]["th_mode"] = sensor_dat[axis_name].th_mode;
      config[axis_name]["cyclic"] = sensor_dat[axis_name].cyclic;
    }
    if (debug) {
      Serial.println("returned_sensor_get_config");
      serializeJsonPretty(config, Serial);
      Serial.println("returned_sensor_get_config_end");
    }
  } catch (const std::exception& e) {
    Serial.println("error: get_config");
    Serial.println(e.what());
  }

  return config;
}

void Sensor::set_input_config(JsonObject config, bool debug) {
  if (debug) {
    Serial.println("set_sensor_axis_config");
  }
  for (auto const& pair : config) {
    string axis_name = pair.key().c_str();
    // should likely use getter/setter here
    set_deadband(axis_name, config[axis_name]["deadband"]);
    sensor_dat[axis_name].offset = config[axis_name]["offset"];
    sensor_dat[axis_name].inverted = config[axis_name]["inverted"];
    sensor_dat[axis_name].lmax = config[axis_name]["lmax"];
    sensor_dat[axis_name].lmin = config[axis_name]["lmin"];
    sensor_dat[axis_name].mode = config[axis_name]["mode"];
    sensor_dat[axis_name].th_mode = config[axis_name]["th_mode"];
    sensor_dat[axis_name].cyclic = config[axis_name]["cyclic"];
  }
  if (debug) {
    Serial.println("set_sensor_axis_config_end");
  }
}

void Sensor::monitor_axis(const std::string& axis) {
  // iterate through sensor_dat, set the ws_monitor flag to true for the
  // specified axis and false for all others

  for (auto& dat : sensor_dat) {
    string axis_name = dat.first;
    if (axis_name == axis) {
      log_i("set monitor_axis: %s", axis.c_str());
      dat.second.ws_monitor = true;
    } else {
      dat.second.ws_monitor = false;
    }
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

bool Sensor::get_inverted(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].inverted;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_deadband(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].deadband;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

float Sensor::get_offset(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].offset;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

/**
@brief get the value of the axis
 */
float Sensor::get_value(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].value_ready;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

/**
 @brief get the value of the sensor clipped to min/max
 */
float Sensor::get_value_constrained(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    // wrap value for circular axis, clip for others.
    if (axis == "pitch" || axis == " yaw" || axis == "roll") {
      float range = sensor_dat[axis].lmax - sensor_dat[axis].lmin;
      if (sensor_dat[axis].value_ready < sensor_dat[axis].lmin) {
        return sensor_dat[axis].value_ready + range;
      } else if (sensor_dat[axis].value_ready > sensor_dat[axis].lmax) {
        return sensor_dat[axis].value_ready - range;
      } else {
        return sensor_dat[axis].value_ready;
      }
    } else {
      if (sensor_dat[axis].value_ready < sensor_dat[axis].lmin) {
        return sensor_dat[axis].lmin;
      } else if (sensor_dat[axis].value_ready > sensor_dat[axis].lmax) {
        return sensor_dat[axis].lmax;
      } else {
        return sensor_dat[axis].value_ready;
      }
    }
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
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    if (sensor_dat[axis].inverted == false) {
      return sensor_dat[axis].bool_value;
    } else {
      return !sensor_dat[axis].bool_value;
    }
  } else {
    throw std::invalid_argument("Axis not found: " + axis);
  }
}

//Setters

// void Sensor::set_enabled(const std::string& axis, bool value) {
//     if(sensor_dat.find(axis) != sensor_dat.end())
//         sensor_dat[axis].enabled = value;
//     else
//         throw std::invalid_argument("Axis not found: " + axis);
// }

void Sensor::set_inverted(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].inverted = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

void Sensor::set_deadband(const std::string& axis, float value) {
  if (sensor_dat.find(axis) != sensor_dat.end()) {
    sensor_dat[axis].deadband = value;
    sensor_dat[axis].NeutralFilter.setDeadband(value);
  } else
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

void Sensor::set_cyclic(const std::string& axis, bool value) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    sensor_dat[axis].cyclic = value;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

bool Sensor::get_cyclic(const std::string& axis) {
  if (sensor_dat.find(axis) != sensor_dat.end())
    return sensor_dat[axis].cyclic;
  else
    throw std::invalid_argument("Axis not found: " + axis);
}

// Offset measurement completion methods
bool Sensor::is_offset_measurement_complete() {
  return offset_measurement_complete;
}

void Sensor::clear_completion_flag() {
  offset_measurement_complete = false;
  // Reset measurement type flags after client has retrieved the offsets
  measure_all = false;
  measure_list = false;
  channels_to_measure.clear();
}

JsonDocument Sensor::get_measured_offsets() {
  JsonDocument result;

  if (measure_all) {
    // Return all channel offsets
    for (const auto& pair : sensor_dat) {
      result[pair.first] = pair.second.offset;
    }
  } else if (measure_list) {
    // Return selected channel offsets
    for (const string& channel : channels_to_measure) {
      if (sensor_dat.find(channel) != sensor_dat.end()) {
        result[channel] = sensor_dat[channel].offset;
      }
    }
  } else {
    // Return single axis offset
    if (sensor_dat.find(axis_to_measure_offset) != sensor_dat.end()) {
      result[axis_to_measure_offset] =
          sensor_dat[axis_to_measure_offset].offset;
    }
  }

  return result;
}
