#ifndef ENGINE_H
#define ENGINE_H

#include <Arduino.h>
#include <unordered_map>
#include "HW_CONFIG.h"
#include "midi/midi_translator.h"
#include "hid/usb_hid.h"
#include "hid/hid_translator.h"
#include "midi/midi_io.h"
#include <ArduinoJson.h>
#include "utils/fs_tools.h"
#include "sensors/sensors.h"
#include "hw_ui.h"
#include "osc/osc_handler.h"
#include "osc/osc_translators.h"

using namespace std;

// this class takes care of the translation of the sensor data to the different outputs
// Todo: maps should not be declared per axis, but per function since some output might rely on 2 inputs

class OSC_handler;  // why do I need forward declaration here??

class Engine {
 public:
  Engine() {
    for (const auto& axis : input_sensor.get_sensor_dat_map()) {
      Miditranslators[axis.first] = MidiTranslator();
      Osctranslators[axis.first] = OscTranslator();
      HID_translators[axis.first] = HidTranslator();
    }
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

  void update();
  void midi_processor();
  void hid_processor();
  void osc_processor();

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

extern Engine engine;

#endif  //ENGINE_H