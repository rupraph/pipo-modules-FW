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
#include "shared_flags.h"

#ifdef PIPO_ANALOG
#include "sensors/analog_out.h"
#endif

using namespace std;

// this class takes care of the translation of the sensor data to the different outputs
// Todo: maps should not be declared per axis, but per function since some output might rely on 2 inputs

void sensorTask(void* pvParameters);

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

  void update();
  void midi_processor(string axis_name, float sensor_val, float sensor_min,
                      float sensor_max);
  void hid_processor(string axis_name, float sensor_val, float sensor_min,
                     float sensor_max);
  void osc_processor(string axis_name, float sensor_val, float sensor_min,
                     float sensor_max);

  // config
  JsonDocument get_config(bool debug = false);
  void set_config(JsonObject config, bool debug = false);

  //utils
  float round_to(float value, int decimal);

//special functions (not of axis type)
#ifdef PIPO_MOTION
  void motion_quat_to_osc();
  bool enable_quat_to_osc = false;
  string quat_to_osc_address = "motion/quat";
#endif

 private:
  unordered_map<string, uint8_t> note_val;  // indexed by axis_name, not channel
  unordered_map<string, uint8_t>
      note_val_prev;  // indexed by axis_name, not channel

  unordered_map<string, float> osc_val;
  unordered_map<string, float> osc_val_prev;
};

extern Engine engine;

#endif  //ENGINE_H