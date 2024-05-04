#ifndef ENGINE_H
#define ENGINE_H

#include "acc_sensor.h"
#include <unordered_map>
#include "midi_translator.h"
#include "hid_translator.h"
#include "midi_io.h"
#include "usb_hid.h"
#include "json.hpp"
#include "fs_tools.h"
#include "config.h"

using namespace std;

// this class will process each data stream from the sensor, and depending on the applied settings, process it further, to finally send it on the selected output interface

//decide if miditranslator holds its config per voice, and get the input name for structure
// or if we do the opposite, and we fetch settings... 

// unordered_map<string, midi_translator> midi_map = {
//     {"roll", midi_translator(0, 0, 0)},

extern unordered_map<string, MidiTranslator> Miditranslators;
extern unordered_map<string, HidTranslator> hid_map;
extern unordered_map<string, int> cc_map;



void engine_setup();
void set_default_config();
json engine_get_config();
void engine_set_config(Config& config);
void engine_update(midi_io& midiio,usb_hid& hidio);
void midi_processsor(midi_io& midiio);
void hid_processor(usb_hid& hidio);





#endif //ENGINE_H