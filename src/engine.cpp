#include "engine.h"

// the engine takes the sensor data and outputs it to the selected interfaces
// based on the configuration

Engine engine;

// using the main loop instead to optimize ram usage
// void sensorTask(void* pvParameters) {
//   TickType_t xLastWakeTime = xTaskGetTickCount();
//   const TickType_t xFrequency = pdMS_TO_TICKS(2.5);  // 2.5ms = 400Hz

//   for (;;) {
//     // sensor_task_interval = millis() - lastMillis;
//     // lastMillis = millis();
//     bool datachanged = input_sensor.update();
//     if (datachanged) {
//       engine.update();
//     }
//     // sensor_task_duration = millis() - lastMillis;
//     // #ifdef PIPO_ANALOG
//     //     analog_out.update();  // should be in seperate task
//     // #endif
//     vTaskDelayUntil(&xLastWakeTime, xFrequency);  // Fixed 400Hz rate
//   }
// }

//Todo: check if processors could access sensor data without having to pass all the arguments so that invert and cyclic could be computed upfront
void Engine::update() {
  if (PAUSED) {
    return;
  }
  if (config.general_config["MidiEnabled"] == true) {
    midiio.manage_sustain();
  }

  // loop through sensor data
  const auto& sensor_dat = input_sensor.get_sensor_dat_map();
  for (auto const& pair : sensor_dat) {
    const string& axis_name = pair.first;
    const SensorDat& dat = pair.second;

    // Read fields directly from SensorDat — no hash lookups
    bool sensor_invert = dat.inverted;
    bool sensor_cycle = dat.cyclic;
    bool sensor_over_out = dat.over_out;
    float sensor_min = dat.lmin;
    float sensor_max = dat.lmax;

    // Inline constrained value logic (was get_value_constrained)
    float sensor_val;
    if (axis_name == "pitch" || axis_name == " yaw" || axis_name == "roll") {
      float range = dat.lmax - dat.lmin;
      if (dat.value_ready < dat.lmin) {
        sensor_val = dat.value_ready + range;
      } else if (dat.value_ready > dat.lmax) {
        sensor_val = dat.value_ready - range;
      } else {
        sensor_val = dat.value_ready;
      }
    } else {
      if (dat.value_ready < dat.lmin) {
        sensor_val = dat.lmin;
      } else if (dat.value_ready > dat.lmax) {
        sensor_val = dat.lmax;
      } else {
        sensor_val = dat.value_ready;
      }
    }

    // Store original values before any transformations
    float original_sensor_val = sensor_val;
    float original_min = sensor_min;
    float original_max = sensor_max;

    // Handle cyclic mode: phase-shifted triangle wave mapping.
    // Phase 0.25 places output 0.5 at the sensor boundary (the discontinuity),
    // so both sides wrap to the same output value with no jump.
    // Phase 0.0 gives the old behaviour (peak at midpoint).
    static const float CYCLIC_PHASE = 0.25f;
    if (sensor_cycle) {
      float range = sensor_max - sensor_min;
      float norm = (sensor_val - sensor_min) / range;  // [0, 1]
      float t = norm - CYCLIC_PHASE + 1.5f;
      t = t - (float)(int)t;  // wrap to [0, 1]
      float tri = 2.0f * t - 1.0f;
      if (tri < 0.0f)
        tri = -tri;                           // triangle wave [0, 1]
      sensor_val = sensor_min + tri * range;  // back to sensor units
    }

    // Apply over_out: if ORIGINAL value exceeds max, return the OUTPUT minimum
    // Check on original value so it works consistently regardless of invert
    if (sensor_over_out && original_sensor_val >= original_max) {
      // The output minimum is sensor_min (which is the logical min considering cyclic)
      sensor_val = sensor_min;
    } else {
      // Apply invert: reverse the value within the range
      if (sensor_invert) {
        // Map value from [min, max] to [max, min]
        sensor_val = sensor_max + sensor_min - sensor_val;
      }
    }

    if (config.general_config["MidiEnabled"] == true &&
        Miditranslators.find(axis_name) != Miditranslators.end()) {
      midi_processor(axis_name, dat, sensor_val, sensor_min, sensor_max);
    }
    if (config.general_config["OSC_ENA"] == true &&
        Osctranslators.find(axis_name) != Osctranslators.end()) {
      osc_processor(axis_name, dat, sensor_val, sensor_min, sensor_max);
    }
    // if (config.general_config["HidEnabled"] == true &&
    //     HID_translators.find(axis_name) != HID_translators.end()) {
    //   hid_processor(axis_name, sensor_val, sensor_min, sensor_max);
    // }
  }

#ifdef PIPO_MOTION
  if (enable_quat_to_osc) {
    motion_quat_to_osc();
  }
#endif
  if (osc.is_enabled() && osc.is_started()) {
    osc.send_bundle();
  }

  // monitor_sensors(sensor);
}

void Engine::midi_processor(const string& axis_name, const SensorDat& dat,
                            float sensor_val, float sensor_min,
                            float sensor_max) {
  MidiTranslator& midi_translator = Miditranslators[axis_name];
  // check if axis is enabled, outside deadzone and not disabled
  int channel = midi_translator.channel;

  if (/*input_sensor.test_outside_deadzone(axis_name) &&*/
      midi_translator.is_enabled() == true) {

    // Read sensor fields directly — no hash lookups
    bool sensor_mode = dat.mode;
    bool sensor_bool = dat.inverted ? !dat.bool_value : dat.bool_value;

    // if CC MODE:
    if (midi_translator.tl_mode == 0) {
      int cc_nb = midi_translator.cc_nb;

      // sensor uses continuous mode
      if (sensor_mode == 0) {
        // Only send when engaged — last CC value holds when not engaged
        if (!dat.engaged && dat.hold_mode)
          return;

        // Todo: hires not tested
        if (midi_translator.get_hires()) {
          uint16_t cc_val =
              max(0, min(midi_translator.get_cc_val(sensor_val, sensor_min,
                                                    sensor_max, 1),
                         16383));

          if (midi_translator.should_send_cc_hires(cc_val)) {
            midiio.sendControlChange(cc_nb, cc_val, channel, true);
          }
        } else {
          uint8_t cc_val =
              max(0, min(midi_translator.get_cc_val(sensor_val, sensor_min,
                                                    sensor_max, 0),
                         127));

          if (midi_translator.should_send_cc(cc_val)) {
            midiio.sendControlChange(cc_nb, cc_val, channel, false);
          }
        }
        // }
      } else  // sensor uses trigger mode
      {

        if (sensor_bool) {
          uint16_t cc_val = midi_translator.get_max_output();
          if (midi_translator.get_hires()) {
            if (midi_translator.should_send_cc_hires(cc_val)) {
              midiio.sendControlChange(cc_nb, cc_val, channel, true);
            }
          } else {
            if (midi_translator.should_send_cc((uint8_t)cc_val)) {
              midiio.sendControlChange(cc_nb, cc_val, channel, false);
            }
          }
        } else {
          uint16_t cc_val = midi_translator.get_min_output();
          if (midi_translator.get_hires()) {
            if (midi_translator.should_send_cc_hires(cc_val)) {
              midiio.sendControlChange(cc_nb, cc_val, channel, true);
            }
          } else {
            if (midi_translator.should_send_cc((uint8_t)cc_val)) {
              midiio.sendControlChange(cc_nb, cc_val, channel, false);
            }
          }
        }
        //vTaskDelay(pdTICKS_TO_MS(5));  // virtually delay cc send. will be
        // solved with task management
      }
    }

    // if Note mode
    else if (midi_translator.tl_mode == 1) {
      int note = (midi_translator.get_note(sensor_val, sensor_min, sensor_max));
      uint8_t current_note = max(0, min(note, 127));  // clip between 0 and 127

      int sustain_ms = int(midi_translator.get_sustain() *
                           1000.0);  // 0 means sustain manager will not
                                     // shutoff note after delay

      // mode is threshold
      if (sensor_mode == 1) {
        int thresh_note = midi_translator.get_root_note();
        // midiio.printNoteList(channel);
        if (sensor_bool) {
          if (  //!midiio.is_note_playing(thresh_note, channel) &&
              dat.trigger_flags.midi_trig) {
            midiio.sendNoteOn(thresh_note, midi_translator.get_velocity(),
                              channel, sustain_ms);
            input_sensor.set_trigger_flag(axis_name, MIDI, false);
          }
        } else {
          // Only send immediate note-off if sustain is 0 (infinite sustain mode)
          // If sustain has a duration, let the sustain manager handle it
          if (sustain_ms == 0) {
            midiio.sendNoteOff(thresh_note, 127, channel);
          }
        }
      } else  // mode is continuous
      {
        // send note on if:
        // sensor in range
        // AND note not already playing
        // AND (note changed OR we entered the range)
        if (dat.engaged &&
            // !midiio.is_note_playing(current_note, channel) &&
            (midi_translator.should_send_note(current_note) ||
             dat.trigger_flags.midi_trig)) {
          midiio.sendNoteOn(current_note, midi_translator.get_velocity(),
                            channel, sustain_ms);
          if (dat.trigger_flags.midi_trig) {
            input_sensor.set_trigger_flag(axis_name, MIDI, false);
          }
        }

        if (dat.untrigger_flags.midi_trig)
        // &&!sensor.is_engaged(axis_name))
        {
          midiio.sendAllNotesOff(channel);
          input_sensor.set_untrigger_flag(axis_name, MIDI, false);
        }
      }

      // #endif
    } else if (midi_translator.tl_mode == 2) {
      if (input_sensor.get_mode(axis_name) == 0) {

        uint16_t pb_val = max(0, min(midi_translator.get_cc_val(
                                         sensor_val, sensor_min, sensor_max, 1),
                                     16383));
        if (midi_translator.should_send_pitch_bend(pb_val)) {
          midiio.sendPitchBend(pb_val, channel);
        }
      }
    }
  }
}

// void Engine::hid_processor(string axis_name, float sensor_val, float sensor_min,
//                            float sensor_max) {

//   HidTranslator& HID_translator = HID_translators[axis_name];
//   bool sensor_bool_val = input_sensor.get_bool_value(axis_name);
//   if (HID_translator.is_enabled() == true) {
//     if (HID_translators.find(axis_name) != HID_translators.end()) {

//       string address = HID_translator.get_map_address();
//       string address2 = HID_translator.get_map_address2();

//       switch ((int)config.general_config["HidMode"]) {
//         case 0:
//           //gamepad mode. to do
//           break;
//         case 1:
//           //continuous mode -> do not update if outise measuring range
//           if (input_sensor.get_mode(axis_name) == false) {
//             if (input_sensor.is_engaged(axis_name)) {
//               hidio.mouse_update(address,
//                                  HID_translator.get_mouse_int(
//                                      sensor_val, sensor_min, sensor_max),
//                                  sensor_bool_val);
//             }
//           } else {
//             hidio.mouse_update(address,
//                                HID_translator.get_mouse_int(
//                                    sensor_val, sensor_min, sensor_max),
//                                sensor_bool_val);
//           }
//           break;
//         case 2:
//           // keyboard mode. only compatible with axis in threshold mode

//           //keystroke mode "once"
//           if (HID_translator.get_stroke_mode() == false) {
//             if (input_sensor.get_trigger_flag(axis_name, HID)) {
//               //Keyboard (it does not allow multiple key presses yet while it could)
//               hidio.keyboard_set_press(address);
//               // hidio.mouse_set_press(address);
//               input_sensor.set_trigger_flag(axis_name, HID, false);
//             }
//           } else {
//             // keystroke mode "maintained".
//             if (!input_sensor.get_threshold_mode(axis_name)) {
//               if (sensor_bool_val) {
//                 hidio.keyboard_set_press(address);
//                 // hidio.mouse_set_press(address);
//               }
//             } else {
//               //deal with 2 key addresses for true/false when basic threshold mode selected
//               //Todo: should not fetch input sensor value here I guess. like other processors
//               if (!sensor_bool_val) {
//                 if (input_sensor.get_value(axis_name) >
//                     input_sensor.get_limit_max(axis_name)) {
//                   hidio.keyboard_set_press(address);
//                 }
//                 if (input_sensor.get_value(axis_name) <
//                     input_sensor.get_limit_min(axis_name)) {
//                   hidio.keyboard_set_press(address2);
//                 }
//                 // hidio.mouse_set_press(address);
//               }
//             }
//           }
//           break;
//       }
//     } else {
//       log_e("Engine: HID key not found");
//     }
//     hidio.update();
//   }
//   hidio.keyboard_release();
// }

void Engine::osc_processor(const string& axis_name, const SensorDat& dat,
                           float sensor_val, float sensor_min,
                           float sensor_max) {
  OscTranslator& osc_translator = Osctranslators[axis_name];
  string address = osc_translator.get_osc_addr();

  if (osc_translator.is_enabled() /* &&
      input_sensor.test_outside_deadzone(axis_name)*/) {

    float new_osc_val;
    if (osc_translator.get_mode_raw()) {
      if (!dat.engaged && dat.hold_mode)
        return;
      new_osc_val = round_to(dat.value_ready, 3);
    } else {
      if (dat.mode == 0) {  // continuous mode
        if (!dat.engaged && dat.hold_mode)
          return;
        new_osc_val = round_to(
            osc_translator.get_value(sensor_val, sensor_min, sensor_max), 3);
        // }
      } else {  // sensor uses trigger mode
        bool sensor_bool = dat.inverted ? !dat.bool_value : dat.bool_value;
        if (sensor_bool) {
          new_osc_val = round_to(osc_translator.get_output_max(), 3);
        } else {
          new_osc_val = round_to(osc_translator.get_output_min(), 3);
        }
      }
    }

    // Check for value change and send if changed
    if (osc_translator.should_send(new_osc_val)) {
      osc.add_to_bundle(address, new_osc_val);
    }
  }
}

float Engine::round_to(float value, int decimal) {
  return round(value * pow(10, decimal)) / pow(10, decimal);
}

void Engine::get_config(JsonDocument& doc, bool debug) {
  doc["engine"].clear();
  for (auto const& pair : Miditranslators) {
    doc["engine"]["engine-midi"][pair.first] = pair.second.get_json();
  }
  // for (auto const& pair : HID_translators) {
  //   doc["engine"]["engine-hid"][pair.first] = pair.second.get_json();
  // }
  for (auto const& pair : Osctranslators) {
    doc["engine"]["engine-osc"][pair.first] = pair.second.get_json();
  }
#ifdef PIPO_MOTION
  doc["engine"]["engine-special"]["quat"]["enabled"] = enable_quat_to_osc;
  doc["engine"]["engine-special"]["quat"]["osc_addr"] = quat_to_osc_address;
#endif
  if (debug) {
    log_d("engine_get_config");
    serializeJsonPretty(doc, Serial);
    log_d("engine_get_config_end");
  }
}

void Engine::set_config(JsonObject configin, bool debug) {
  if (debug) {
    log_d("will set engine config:");
    serializeJsonPretty(configin, Serial);
    log_d("");  // Empty line
  }

  JsonDocument jmidi = configin["engine-midi"];

  // set midi config from main config
  if (debug)
    log_d("set engine midi");
  if (config.general_config["MidiEnabled"] == true) {
    midiio.sendAllNotesOff(
        0);  // stop all notes on all channels before changing config
  }
  for (auto const& pair : Miditranslators) {
    if (jmidi[pair.first].is<JsonVariant>()) {
      // Serial.println(jmidi[pair.first].dump().c_str());
      // Serial.println(pair.first.c_str());
      Miditranslators[pair.first].set_from_json(jmidi[pair.first]);
    }
  }
  jmidi.clear();
  // set hid config from general config
  if (debug)
    log_d("set engine hid");
  // JsonDocument jhid = configin["engine-hid"];
  // for (auto const& pair : HID_translators) {
  //   if (jhid[pair.first].is<JsonVariant>()) {
  //     HID_translators[pair.first].set_from_json(jhid[pair.first]);
  //   }
  // }
  // jhid.clear();
  JsonDocument josc = configin["engine-osc"];
  if (debug)
    log_d("set engine osc");
  for (auto const& pair : Osctranslators) {
    if (josc[pair.first].is<JsonVariant>()) {
      Osctranslators[pair.first].set_from_json(josc[pair.first]);
    }
  }
  josc.clear();
#ifdef PIPO_MOTION
  JsonObject jspecial = configin["engine-special"].as<JsonObject>();
  if (debug)
    log_d("set engine special");
  // check how many elements are in the json object
  if (jspecial.size() > 0) {
    for (JsonPair pair : jspecial) {
      if (pair.key() == "quat") {
        enable_quat_to_osc = jspecial["quat"]["enabled"].as<bool>();
        quat_to_osc_address = jspecial["quat"]["osc_addr"].as<string>();
      }
    }
  } else {
    log_w("Engine: no special config found");
  }
  jspecial.clear();
#endif
  if (debug) {
    log_d("engine config set");
  }
}

#ifdef PIPO_MOTION
void Engine::motion_quat_to_osc() {
  float quats[4];
  if (input_sensor.get_relative_mode() == true) {
    input_sensor.get_relative_quat(quats[0], quats[1], quats[2], quats[3]);
  } else {
    input_sensor.get_quat(quats[0], quats[1], quats[2], quats[3]);
  }

  // Round values to 3 decimals for comparison (consistent with osc_processor)
  float rounded_quats[4];
  for (int i = 0; i < 4; i++) {
    rounded_quats[i] = round_to(quats[i], 3);
  }

  // Only send if any component has changed
  bool changed = false;
  for (int i = 0; i < 4; i++) {
    if (rounded_quats[i] != prev_quat[i]) {
      changed = true;
      prev_quat[i] = rounded_quats[i];
    }
  }

  if (changed) {
    osc.add_to_bundle(quat_to_osc_address + "w", rounded_quats[0]);
    osc.add_to_bundle(quat_to_osc_address + "x", rounded_quats[1]);
    osc.add_to_bundle(quat_to_osc_address + "y", rounded_quats[2]);
    osc.add_to_bundle(quat_to_osc_address + "z", rounded_quats[3]);
  }
}
#endif
