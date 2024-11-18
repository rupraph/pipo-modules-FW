#ifndef ENGINE_H
#define ENGINE_H

#include <Arduino.h>
#include <unordered_map>
#include "midi/midi_translator.h"
#include "hid/usb_hid.h"
#include "hid/hid_translator.h"
#include "midi/midi_io.h"
#include <ArduinoJson.h>
#include "utils/fs_tools.h"
#include "sensor/input_sensor.h"
#include "hw_ui.h"
#include "osc_handler.h"
#include "osc_translators.h"

using namespace std;

// this class takes care of the translation of the sensor data to the different outputs
// Todo: maps should not be declared per axis, but per function since some output might rely on 2 inputs

class OSC_handler;  // why do I need forward declaration here??

class Engine {
 public:
  Engine() {
#if defined(PIPO_MOTION)
    string axis_list[] = {"roll", "pitch", "yaw", "accX", "accY", "accZ"};
    for (auto axis : axis_list) {
      Miditranslators[axis] = MidiTranslator();
      Osctranslators[axis] = OscTranslator();
      HID_translators[axis] = HidTranslator();
    }

#elif defined(PIPO_RANGE)
    Miditranslators = {{"dist", MidiTranslator()}};
    Osctranslators = {{"dist", OscTranslator()}};
    HID_translators = {{"dist", HidTranslator()}};

#elif defined(PIPO_ANALOG)
    string axis_list[] = {"A1", "A2", "A3", "A4", "A5", "A6",
                          "T1", "T2", "T3", "T4", "T5", "T6"};
    for (auto axis : axis_list) {
      Miditranslators[axis] = MidiTranslator();
      Osctranslators[axis] = OscTranslator();
      HID_translators[axis] = HidTranslator();
    }
#endif
  }

  unordered_map<string, MidiTranslator> Miditranslators;
  unordered_map<string, OscTranslator> Osctranslators;
  unordered_map<string, HidTranslator> HID_translators;

  hid_gamepad_report_t gp;
  hid_keyboard_report_t kb;
  hid_mouse_report_t mouse;

  bool get_paused() { return paused; }
  void set_paused(bool value) { paused = value; }
  void toggle_pause();

  void update(Sensor& sensor, midi_io& midiio, usb_hid& hidio,
              OSC_handler& osc);
  void midi_processor(Sensor& sensor, midi_io& midiio);
  void hid_processor(Sensor& sensor, usb_hid& hidio);
  void osc_processor(Sensor& sensor, OSC_handler& osc);

  // config
  JsonDocument get_config(bool debug = false);
  void set_config(JsonObject config, bool debug = false);

  //utils
  float round_to(float value, int decimal);

 private:
  uint8_t note_val[128];
  uint8_t note_val_prev[128];
  bool paused = false;

  unordered_map<string, float> osc_val;
  unordered_map<string, float> osc_val_prev;
};

#endif  //ENGINE_H