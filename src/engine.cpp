#include "engine.h"
#include "HW_CONFIG.h"

using json = nlohmann::json;

// the engine takes the sensor data and outputs it to the selected interfaces
// based on the configuration

// Todo engine.
// could use combination mode to have note from orientation, and trigger from
// acceleration

void Engine::update(Sensor& sensor, midi_io& midiio, usb_hid& hidio,
                    OSC_handler& osc) {
  if (config.general_config["MidiEnabled"] == true) {
    midiio.manage_sustain();
    midi_processor(sensor, midiio);
  }
  if (config.general_config["OSC_ENA"] == true) {
    osc_processor(sensor, osc);
  }
  //if (config.general_config["HidEnabled"] == true) {
  hid_processor(sensor, hidio);
  //}
}

void Engine::midi_processor(Sensor& sensor, midi_io& midiio) {
  // loop through sensor data
  const auto& sensor_dat = sensor.get_sensor_dat_map();
  for (auto const& pair : sensor_dat) {
    string axis_name = pair.first;
    MidiTranslator Midi_translator = Miditranslators[axis_name];

    float sensor_val = sensor.get_value(axis_name);
    float sensor_min = sensor.get_limit_min(axis_name);
    float sensor_max = sensor.get_limit_max(axis_name);
    int channel = Midi_translator.channel;

    // check if axis is enabled, outside deadzone and not disabled
    if (sensor.test_outside_deadzone(axis_name) &&
        Midi_translator.enabled == true) {
      // if CC MODE
      if (Midi_translator.translator_mode == 0) {
        int cc_number = Midi_translator.cc_number;

        // sensor uses continuous mode
        if (sensor.get_mode(axis_name) == 0) {
          if (sensor.is_within_range(axis_name)) {
            // Todo: hires not tested
            if (Midi_translator.getHires()) {
              uint16_t cc_val =
                  max(0, min(Midi_translator.get_cc_val(sensor_val, sensor_min,
                                                        sensor_max, 1),
                             16383));

              midiio.sendControlChange(cc_number, cc_val, channel, true);
            } else {
              uint8_t cc_val =
                  max(0, min(Midi_translator.get_cc_val(sensor_val, sensor_min,
                                                        sensor_max, 0),
                             127));
              midiio.sendControlChange(cc_number, cc_val, channel, false);
            }
          }
        } else  // sensor uses trigger mode
        {
          if (sensor.get_bool_value(axis_name)) {
            uint16_t cc_val = Midi_translator.getMaxOutput();
            if (Midi_translator.getHires()) {
              midiio.sendControlChange(cc_number, cc_val, channel, true);
            } else {
              midiio.sendControlChange(cc_number, cc_val, channel, false);
            }
          } else {
            uint16_t cc_val = Midi_translator.getMinOutput();
            if (Midi_translator.getHires()) {
              midiio.sendControlChange(cc_number, cc_val, channel, true);
            } else {
              midiio.sendControlChange(cc_number, cc_val, channel, false);
            }
          }
          vTaskDelay(pdTICKS_TO_MS(5));  // virtually delay cc send. will be
                                         // solved with task management
        }
      }

      // if Note mode
      else {
        // getting note for continuous mode
        note_val_prev[channel] = note_val[channel];
        int note =
            (Midi_translator.get_note(sensor_val, sensor_min, sensor_max));
        note_val[channel] = max(0, min(note, 127));  // clip between 0 and 127

        int sustain_ms = int(Midi_translator.getSustain() *
                             1000.0);  // 0 means sustain manager will not
                                       // shutoff note after delay

        // probaly get triggered should be something linked to the deadzone
        // #if defined(PIPO_ANALOG)
        // trigger new note if within range, not already playing, and new note
        // is different from previous note if (sensor.is_within_range(axis_name)
        // && note_val[channel]!=note_val_prev[channel]
        // && !midiio.is_note_playing(note_val[channel],channel))
        // {
        //     midiio.sendNoteOn(note_val[channel],127,channel,3000);
        // }

        // //if entering range, send new note
        // if (sensor.get_triggered(axis_name))
        // {
        //     midiio.sendNoteOn(note_val[channel],127,channel);
        //     sensor.set_triggered(axis_name,false);
        // }

        // //if exiting range, send note off
        // if (sensor.get_untriggered(axis_name))
        // {
        //     midiio.sendAllNotesOff(channel);
        //     sensor.set_untriggered(axis_name,false);
        // }
        // #endif

        // # if defined(PIPO_MOTION)
        // if (note_val[channel]!=note_val_prev[channel])
        // {
        //     midiio.sendNoteOn(note_val[channel],127,channel,5000);
        // }
        // #endif

        // #if defined(PIPO_RANGE) || defined(PIPO_ANALOG)

        // mode is threshold
        if (sensor.get_mode(axis_name) == 1) {
          int thresh_note = Midi_translator.getRootNote();
          // midiio.printNoteList(channel);
          if (sensor.get_bool_value(axis_name)) {
            if (!midiio.is_note_playing(thresh_note, channel) &&
                sensor.get_triggered(axis_name)) {
              midiio.sendNoteOn(thresh_note, 127, channel, sustain_ms);
              sensor.set_triggered(axis_name, false);
            }
          } else {
            midiio.sendAllNotesOff(channel);
          }
        } else  // mode is continuous
        {
          if (sensor.is_within_range(axis_name) &&
              !midiio.is_note_playing(note_val[channel], channel) &&
              (note_val[channel] != note_val_prev[channel] ||
               sensor.get_triggered(axis_name))) {
            midiio.sendNoteOn(note_val[channel], 127, channel, sustain_ms);
            if (sensor.get_triggered(axis_name)) {
              sensor.set_triggered(axis_name, false);
            }
          }

          if (sensor.get_untriggered(axis_name))
          // &&!sensor.is_within_range(axis_name))
          {
            midiio.sendAllNotesOff(channel);
            sensor.set_untriggered(axis_name, false);
          }
        }

        // #endif
      }
    }
  }
}

void Engine::hid_processor(Sensor& sensor, usb_hid& hidio) {

  const auto& sensor_dat = sensor.get_sensor_dat_map();

  for (auto const& pair : sensor_dat) {
    string axis_name = pair.first;
    float sensor_val = sensor.get_value(axis_name);
    float sensor_min = sensor.get_limit_min(axis_name);
    float sensor_max = sensor.get_limit_max(axis_name);

    //keyboard
    //hidio.keyboard_release();
    if (HID_translators[axis_name].get_enabled() == true)  // &&
    // hidio.get_hid_mode() == 2)
    {
      if (HID_translators.find(axis_name) != HID_translators.end()) {
        string map_address = HID_translators[axis_name].get_map_address();
        char key = 'b';  //map_address[4];
        //uint8_t keycode = hidio.kb_ascii_to_code[0][0];

        //maybe key should be send until it is released, trigger is for debug
        if (sensor.get_triggered(axis_name)) {
          Serial.println("key pressed");
          Serial.println(key);
          //hidio.keyboard_update();
          hidio.keyboard_set_press('b');
          sensor.set_triggered(axis_name, false);
        }
        if (sensor.get_untriggered(axis_name)) {}
      } else {
        Serial.println("key not found");
      }
    }
    hidio.keyboard_release();
  }
}

void Engine::osc_processor(Sensor& sensor, OSC_handler& osc) {
  // Todo: loop through sensor data -> indentical for 3 processor, should be
  // factorized

  const auto& sensor_dat = sensor.get_sensor_dat_map();
  for (auto const& pair : sensor_dat) {
    string axis_name = pair.first;
    float sensor_val = sensor.get_value(axis_name);
    OscTranslator Osc_translator = Osctranslators[axis_name];

    if (Osc_translator.enabled && sensor.test_outside_deadzone(axis_name)) {
      float sensor_min = sensor.get_limit_min(axis_name);
      float sensor_max = sensor.get_limit_max(axis_name);
      osc_val_prev[axis_name] = osc_val[axis_name];
      if (sensor.get_mode(axis_name) == 0) {  // continuous mode

        if (sensor.is_within_range(axis_name)) {
          osc_val[axis_name] = round_to(
              Osc_translator.get_value(sensor_val, sensor_min, sensor_max), 2);

          if (osc_val[axis_name] != osc_val_prev[axis_name]) {
            osc.sendOscMessage(axis_name, osc_val[axis_name]);
          }
        }
      } else  // sensor uses trigger mode
      {
        if (sensor.get_bool_value(axis_name)) {
          osc_val[axis_name] = round_to(Osc_translator.get_output_max(), 2);
          if (osc_val[axis_name] != osc_val_prev[axis_name]) {
            osc.sendOscMessage(axis_name, osc_val[axis_name]);
          }
        } else {
          osc_val[axis_name] = round_to(Osc_translator.get_output_min(), 2);
          if (osc_val[axis_name] != osc_val_prev[axis_name]) {
            osc.sendOscMessage(axis_name, osc_val[axis_name]);
          }
        }
      }
    }
  }
}

float Engine::round_to(float value, int decimal) {
  return round(value * pow(10, decimal)) / pow(10, decimal);
}

json Engine::get_config(bool debug) {
  json j;
  for (auto const& pair : Miditranslators) {
    j["engine-midi"][pair.first] = pair.second.get_json();
  }
  for (auto const& pair : HID_translators) {
    j["engine-hid"][pair.first] = pair.second.get_json();
  }
  for (auto const& pair : Osctranslators) {
    j["engine-osc"][pair.first] = pair.second.get_json();
  }
  if (debug) {
    Serial.println(F("engine_get_config"));
    Serial.println(j.dump(4).c_str());
    Serial.println("engine_get_config_end");
  }

  return j;
}

void Engine::set_config(json& config, bool debug) {
  if (debug) {
    Serial.println("will set config");
    Serial.println(config.dump().c_str());
    Serial.println();
  }

  json jmidi = config["engine-midi"];

  // set midi config from main config
  for (auto const& pair : Miditranslators) {
    if (jmidi.find(pair.first) != jmidi.end()) {
      // Serial.println(jmidi[pair.first].dump().c_str());
      // Serial.println(pair.first.c_str());
      Miditranslators[pair.first].set_from_json(jmidi[pair.first]);
    }
  }
  Serial.println("midi config set");
  // set hid config from general config
  json jhid = config["engine-hid"];
  for (auto const& pair : HID_translators) {
    if (jhid.find(pair.first) != jhid.end()) {
      HID_translators[pair.first].set_from_json(jhid[pair.first]);
    }
  }
  Serial.println("hid config set");

  json josc = config["engine-osc"];
  for (auto const& pair : Osctranslators) {
    if (josc.find(pair.first) != josc.end()) {
      Osctranslators[pair.first].set_from_json(josc[pair.first]);
    }
  }
  Serial.println("osc config set");
}
