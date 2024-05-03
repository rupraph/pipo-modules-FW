#ifndef ENGINE_H
#define ENGINE_H

#include "acc_sensor.h"
#include <unordered_map>
#include "midi_translator.h"
#include "midi_io.h"
#include "usb_hid.h"
#include "json.hpp"
#include "fs_tools.h"

using namespace std;

// this class will process each data stream from the sensor, and depending on the applied settings, process it further, to finally send it on the selected output interface

//decide if miditranslator holds its config per voice, and get the input name for structure
// or if we do the opposite, and we fetch settings... 

// unordered_map<string, midi_translator> midi_map = {
//     {"roll", midi_translator(0, 0, 0)},

extern unordered_map<string, MidiTranslator> Miditranslators;
extern unordered_map<string, int> cc_map;



void engine_setup();
void set_default_config();
string get_config();
void load_config(String path);
void set_config(string data);
void engine_update(midi_io& midiio);
void midi_processsor(midi_io& midiio);
void hid_processor();





#endif //ENGINE_H