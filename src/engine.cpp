#include "engine.h"

// the engine takes the sensor data and outputs it to the selected interfaces
// based on the configuration

// Todo engine.
// could use combination mode to have note from orientation, and trigger from
// acceleration
Engine engine;

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
    string axis_name = pair.first;
    // Serial.println(axis_name.c_str());
    float sensor_val = input_sensor.get_value_constrained(
        axis_name);  // could add invert here so that I get the inverted value here.
    bool sensor_invert = input_sensor.get_inverted(axis_name);
    bool sensor_cycle = input_sensor.get_cyclic(axis_name);
    float sensor_min = input_sensor.get_limit_min(axis_name);
    float sensor_max = input_sensor.get_limit_max(axis_name);
    float sensor_midpoint;

    sensor_midpoint = sensor_min + (sensor_max - sensor_min) / 2.0f;
    if (sensor_cycle) {
      if (sensor_val < sensor_midpoint) {
        sensor_max = sensor_midpoint;
      } else {
        sensor_min = sensor_max;
        sensor_max = sensor_midpoint;
      }
    }

    if (sensor_invert == true) {
      float temp = sensor_max;
      sensor_max = sensor_min;
      sensor_min = temp;
    }

    if (config.general_config["MidiEnabled"] == true &&
        Miditranslators.find(axis_name) != Miditranslators.end()) {
      midi_processor(axis_name, sensor_val, sensor_min, sensor_max);
    }
    if (config.general_config["OSC_ENA"] == true &&
        Osctranslators.find(axis_name) != Osctranslators.end()) {
      osc_processor(axis_name, sensor_val, sensor_min, sensor_max);
    }
    if (config.general_config["HidEnabled"] == true &&
        HID_translators.find(axis_name) != HID_translators.end()) {
      hid_processor(axis_name, sensor_val, sensor_min, sensor_max);
    }
  }
#ifdef PIPO_MOTION
  if (enable_quat_to_osc) {
    motion_quat_to_osc();
  }
#endif
  // monitor_sensors(sensor);
}

void Engine::midi_processor(string axis_name, float sensor_val,
                            float sensor_min, float sensor_max) {
  MidiTranslator& midi_translator = Miditranslators[axis_name];
  // check if axis is enabled, outside deadzone and not disabled
  int channel = midi_translator.channel;

  if (input_sensor.test_outside_deadzone(axis_name) &&
      midi_translator.is_enabled() == true) {
    // Serial.print("min:");
    // Serial.print(sensor_min);
    // Serial.print(" max:");
    // Serial.print(sensor_max);
    // Serial.print("val");
    // Serial.println(sensor_val);
    // if CC MODE:
    if (midi_translator.tl_mode == 0) {
      int cc_nb = midi_translator.cc_nb;

      // sensor uses continuous mode
      if (input_sensor.get_mode(axis_name) == 0) {
        // if (input_sensor.is_within_range(axis_name)) {
        // Todo: hires not tested
        if (midi_translator.get_hires()) {
          uint16_t cc_val =
              max(0, min(midi_translator.get_cc_val(sensor_val, sensor_min,
                                                    sensor_max, 1),
                         16383));

          midiio.sendControlChange(cc_nb, cc_val, channel, true);
        } else {
          uint8_t cc_val =
              max(0, min(midi_translator.get_cc_val(sensor_val, sensor_min,
                                                    sensor_max, 0),
                         127));

          // Serial.println(cc_val);
          midiio.sendControlChange(cc_nb, cc_val, channel, false);
          //Serial.println(sensor_min);
        }
        // }
      } else  // sensor uses trigger mode
      {

        if (input_sensor.get_bool_value(axis_name)) {
          uint16_t cc_val = midi_translator.get_max_output();
          if (midi_translator.get_hires()) {
            midiio.sendControlChange(cc_nb, cc_val, channel, true);
          } else {
            midiio.sendControlChange(cc_nb, cc_val, channel, false);
          }
        } else {
          uint16_t cc_val = midi_translator.get_min_output();
          if (midi_translator.get_hires()) {
            midiio.sendControlChange(cc_nb, cc_val, channel, true);
          } else {
            midiio.sendControlChange(cc_nb, cc_val, channel, false);
          }
        }
        //vTaskDelay(pdTICKS_TO_MS(5));  // virtually delay cc send. will be
        // solved with task management
      }
    }

    // if Note mode
    else {
      // getting note for continuous mode
      note_val_prev[channel] = note_val[channel];
      int note = (midi_translator.get_note(sensor_val, sensor_min, sensor_max));
      note_val[channel] = max(0, min(note, 127));  // clip between 0 and 127

      int sustain_ms = int(midi_translator.get_sustain() *
                           1000.0);  // 0 means sustain manager will not
                                     // shutoff note after delay

      // mode is threshold
      if (input_sensor.get_mode(axis_name) == 1) {
        int thresh_note = midi_translator.get_root_note();
        // midiio.printNoteList(channel);
        if (input_sensor.get_bool_value(axis_name)) {
          if (  //!midiio.is_note_playing(thresh_note, channel) &&
              input_sensor.get_trigger_flag(axis_name, MIDI)) {
            midiio.sendNoteOn(thresh_note, midi_translator.get_velocity(),
                              channel, sustain_ms);
            input_sensor.set_trigger_flag(axis_name, MIDI, false);
          }
        } else {
          midiio.sendNoteOff(thresh_note, 127, channel);
        }
      } else  // mode is continuous
      {
        // send note on if:
        // sensor in range
        // AND note not already playing
        // AND (note is diff from previous OR we entered the range)
        if (input_sensor.is_within_range(axis_name) &&
            // !midiio.is_note_playing(note_val[channel], channel) &&
            (note_val[channel] != note_val_prev[channel] ||
             input_sensor.get_trigger_flag(axis_name, MIDI))) {
          midiio.sendNoteOn(note_val[channel], midi_translator.get_velocity(),
                            channel, sustain_ms);
          if (input_sensor.get_trigger_flag(axis_name, MIDI)) {
            input_sensor.set_trigger_flag(axis_name, MIDI, false);
          }
        }

        if (input_sensor.get_untrigger_flag(axis_name, MIDI))
        // &&!sensor.is_within_range(axis_name))
        {
          midiio.sendAllNotesOff(channel);
          input_sensor.set_untrigger_flag(axis_name, MIDI, false);
        }
      }

      // #endif
    }
  }
}

void Engine::hid_processor(string axis_name, float sensor_val, float sensor_min,
                           float sensor_max) {

  HidTranslator& HID_translator = HID_translators[axis_name];
  bool sensor_bool_val = input_sensor.get_bool_value(axis_name);
  if (HID_translator.is_enabled() == true) {
    if (HID_translators.find(axis_name) != HID_translators.end()) {

      string address = HID_translator.get_map_address();
      string address2 = HID_translator.get_map_address2();

      switch ((int)config.general_config["HidMode"]) {
        case 0:
          //gamepad mode. to do
          break;
        case 1:
          //continuous mode -> do not update if outise measuring range
          if (input_sensor.get_mode(axis_name) == false) {
            if (input_sensor.is_within_range(axis_name)) {
              hidio.mouse_update(address,
                                 HID_translator.get_mouse_int(
                                     sensor_val, sensor_min, sensor_max),
                                 sensor_bool_val);
            }
          } else {
            hidio.mouse_update(address,
                               HID_translator.get_mouse_int(
                                   sensor_val, sensor_min, sensor_max),
                               sensor_bool_val);
          }
          break;
        case 2:
          // keyboard mode. only compatible with axis in threshold mode

          //keystroke mode "once"
          if (HID_translator.get_stroke_mode() == false) {
            if (input_sensor.get_trigger_flag(axis_name, HID)) {
              //Keyboard (it does not allow multiple key presses yet while it could)
              hidio.keyboard_set_press(address);
              // hidio.mouse_set_press(address);
              input_sensor.set_trigger_flag(axis_name, HID, false);
            }
          } else {
            // keystroke mode "maintained".
            if (!input_sensor.get_threshold_mode(axis_name)) {
              if (sensor_bool_val) {
                hidio.keyboard_set_press(address);
                // hidio.mouse_set_press(address);
              }
            } else {
              //deal with 2 key addresses for true/false when basic threshold mode selected
              //Todo: should not fetch input sensor value here I guess. like other processors
              if (!sensor_bool_val) {
                if (input_sensor.get_value(axis_name) >
                    input_sensor.get_limit_max(axis_name)) {
                  hidio.keyboard_set_press(address);
                }
                if (input_sensor.get_value(axis_name) <
                    input_sensor.get_limit_min(axis_name)) {
                  hidio.keyboard_set_press(address2);
                }
                // hidio.mouse_set_press(address);
              }
            }
          }
          break;
      }
    } else {
      Serial.println("key not found");
    }
    hidio.update();
  }
  hidio.keyboard_release();
}

void Engine::osc_processor(string axis_name, float sensor_val, float sensor_min,
                           float sensor_max) {
  // Todo: loop through sensor data -> indentical for 3 processor, should be
  // factorized

  OscTranslator& osc_translator = Osctranslators[axis_name];
  string address = osc_translator.get_osc_addr();

  if (osc_translator.is_enabled() &&
      input_sensor.test_outside_deadzone(axis_name)) {

    osc_val_prev[axis_name] = osc_val[axis_name];
    if (input_sensor.get_mode(axis_name) == 0) {  // continuous mode

      // if (input_sensor.is_within_range(axis_name)) {
      osc_val[axis_name] = round_to(
          osc_translator.get_value(sensor_val, sensor_min, sensor_max), 3);

      if (osc_val[axis_name] != osc_val_prev[axis_name]) {
        osc.send_osc_message(address, osc_val[axis_name]);
      }
      // }
    } else  // sensor uses trigger mode
    {
      if (input_sensor.get_bool_value(axis_name)) {
        osc_val[axis_name] = round_to(osc_translator.get_output_max(), 3);
        if (osc_val[axis_name] != osc_val_prev[axis_name]) {
          osc.send_osc_message(address, osc_val[axis_name]);
        }
      } else {
        osc_val[axis_name] = round_to(osc_translator.get_output_min(), 3);
        if (osc_val[axis_name] != osc_val_prev[axis_name]) {
          osc.send_osc_message(address, osc_val[axis_name]);
        }
      }
    }
  }
}

float Engine::round_to(float value, int decimal) {
  return round(value * pow(10, decimal)) / pow(10, decimal);
}

JsonDocument Engine::get_config(bool debug) {
  JsonDocument j;
  for (auto const& pair : Miditranslators) {
    j["engine-midi"][pair.first] = pair.second.get_json();
  }
  for (auto const& pair : HID_translators) {
    j["engine-hid"][pair.first] = pair.second.get_json();
  }
  for (auto const& pair : Osctranslators) {
    j["engine-osc"][pair.first] = pair.second.get_json();
  }
#ifdef PIPO_MOTION
  j["engine-special"]["quat"]["enabled"] = enable_quat_to_osc;
  j["engine-special"]["quat"]["osc_addr"] = quat_to_osc_address;
#endif
  if (debug) {
    Serial.println("engine_get_config");
    serializeJsonPretty(j, Serial);
    Serial.println("engine_get_config_end");
  }

  return j;
}

void Engine::set_config(JsonObject config, bool debug) {
  if (debug) {
    Serial.println("will set engine config:");
    serializeJsonPretty(config, Serial);
    Serial.println();
  }

  JsonDocument jmidi = config["engine-midi"];

  // set midi config from main config
  if (debug)
    Serial.println("set engine midi");
  for (auto const& pair : Miditranslators) {
    if (jmidi[pair.first].is<JsonVariant>()) {
      // Serial.println(jmidi[pair.first].dump().c_str());
      // Serial.println(pair.first.c_str());
      Miditranslators[pair.first].set_from_json(jmidi[pair.first]);
    }
  }
  // set hid config from general config
  if (debug)
    Serial.println("set engine hid");
  JsonDocument jhid = config["engine-hid"];
  for (auto const& pair : HID_translators) {
    if (jhid[pair.first].is<JsonVariant>()) {
      HID_translators[pair.first].set_from_json(jhid[pair.first]);
    }
  }
  JsonDocument josc = config["engine-osc"];
  if (debug)
    Serial.println("set engine osc");
  for (auto const& pair : Osctranslators) {
    if (josc[pair.first].is<JsonVariant>()) {
      Osctranslators[pair.first].set_from_json(josc[pair.first]);
    }
  }
#ifdef PIPO_MOTION
  JsonObject jspecial = config["engine-special"].as<JsonObject>();
  if (debug)
    Serial.println("set engine special");
  // check how many elements are in the json object
  if (jspecial.size() > 0) {
    for (JsonPair pair : jspecial) {
      if (pair.key() == "quat") {
        enable_quat_to_osc = jspecial["quat"]["enabled"].as<bool>();
        quat_to_osc_address = jspecial["quat"]["osc_addr"].as<string>();
      }
    }
  } else {
    Serial.println("no special config found");
  }
#endif
  if (debug) {
    Serial.println("engine config set");
  }
}
#ifdef PIPO_MOTION
void Engine::motion_quat_to_osc() {
  float quats[4];
  input_sensor.get_quat(quats[0], quats[1], quats[2], quats[3]);
  osc.send_osc_message(quat_to_osc_address + "w", quats[0]);
  osc.send_osc_message(quat_to_osc_address + "x", quats[1]);
  osc.send_osc_message(quat_to_osc_address + "y", quats[2]);
  osc.send_osc_message(quat_to_osc_address + "z", quats[3]);
}
#endif
