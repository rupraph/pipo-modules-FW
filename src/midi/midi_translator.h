#ifndef MIDI_TRANSLATOR_H
#define MIDI_TRANSLATOR_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <MIDI.h>
#include <ArduinoJson.h>

using namespace std;

//This class is used to convert sensor value to usable midi messages

class MidiTranslator

// Todo:
// move all members to private and write methods to public
// change all set/get in engine
// add option to accomodate full turn ie back to beginning at end of range to avoid sawtooth
// replace limit_max by a range. propagate to Hid ?

{
 public:
  MidiTranslator();
  bool enabled = false;
  int channel = 1;  // should be in engine.
  int cc_nb = 1;

  //notes variables
  int tl_mode = 0;  //0=cc, 1 note, 2 both

  string pattern = "scale";  //can be scale, arpegio or interval (note interval
  string scaleType = "major";  // see maps below for keywords
  int rootNote = 45;
  int nbOfNotes = 25;
  vector<int> current_scale;
  float sustain = 1.0;  //in seconds. 0 for infinite

  //CC variables
  int cc_max;
  int cc_min = 0;
  // int interpolation_type = 0;  //0=linear, 1=step, 2=log
  bool hires = false;

  // Notes scale methods
  int get_note(float value, float min_input, float max_input);
  void print_scale(vector<int> scale);

  int convert_note_name_to_number(string noteName);
  string convert_number_to_note_name(int noteNumber);
  bool is_a_note(string noteName);
  vector<string> get_scale_names();
  void update_scale();

  //cc methods
  int get_cc_val(float value, float min_input, float max_input,
                 bool hires = false);
  int map_linear(float x, float min_input, float max_input);

  // save/load
  JsonDocument get_json() const;
  void set_from_json(const JsonDocument& j);

  // Getter setters
  bool get_hires() const;
  void set_hires(bool h);

  int get_channel();
  void set_channel(int c);

  int get_cc_number();
  void set_cc_number(int c);

  int get_translator_mode();
  void set_translator_mode(int t);

  string get_scale_type();
  void set_scale_type(string scaleType);

  int get_root_note();
  void set_root_note(string rootNote);
  void set_root_note(int notenb);

  int get_number_of_notes();
  void set_number_of_notes(int nbOfNotes);

  float get_sustain();
  void set_sustain(float s);

  int get_max_output();
  void set_max_output(int m);

  int get_min_output();
  void set_min_output(int m);

  int get_interpolation_type();
  void set_interpolation_type(int i);

  bool get_enabled();
  void set_enabled(bool e);

  // template <typename T>
  // void set_param(const string& param_name, const T& value) {
  //     if (param_name == "tl_mode") {
  //         tl_mode = value;
  //     }
  //     else if (param_name == "channel") {
  //         channel = value;
  //     }
  //     else if (param_name == "cc_nb") {
  //         cc_nb = value;
  //     }
  //     else if (param_name == "scaleType") {
  //         scaleType = value;
  //     }
  //     else if (param_name == "rootNote") {
  //         rootNote = value;
  //     }
  //     else if (param_name == "nbOfNotes") {
  //         nbOfNotes = value;
  //     }
  //     else if (param_name == "max_output") {
  //         max_output = value;
  //     }
  //     else if (param_name == "min_output") {
  //         min_output = value;
  //     }
  //     else if (param_name == "interpolation_type") {
  //         interpolation_type = value;
  //     }
  //     else if (param_name == "hires") {
  //         hires = value;
  //     }
  //     else if (param_name == "enabled") {
  //         enabled = value;
  //     }
  //     else {
  //         Serial.println("Error: unknown parameter name");
  //     }
  // }

  // Notes scale variables
  // could be replaced by enums to speed up things
  // Todo: add arpegios // chords
  unordered_map<string, vector<int>> scales = {
      {"major", {0, 2, 4, 5, 7, 9, 11}},
      {"minor", {0, 2, 3, 5, 7, 8, 10}},
      {"minor pentatonic", {0, 3, 5, 6, 10}},
      {"major pentatonic", {0, 2, 4, 7, 9}},
      {"blues minor", {0, 3, 5, 6, 7, 10}},
      {"blues major", {0, 2, 3, 5, 6, 7}},
      {"chromatic", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}},
      {"whole tone", {0, 2, 4, 6, 8, 10}},
      {"octatonic", {0, 1, 3, 4, 6, 7, 9, 10}},
      {"diatonic", {0, 2, 4, 5, 7, 9, 11}},
      {"harmonic minor", {0, 2, 3, 5, 7, 8, 11}},
      {"melodic minor", {0, 2, 3, 5, 7, 9, 11}},
      {"dorian", {0, 2, 3, 5, 7, 9, 10}},
      {"phrygian", {0, 1, 3, 5, 7, 8, 10}},
      {"lydian", {0, 2, 4, 6, 7, 9, 11}},
      {"mixolydian", {0, 2, 4, 5, 7, 9, 10}},
      {"locrian", {0, 1, 3, 5, 6, 8, 10}},
      {"ionian", {0, 2, 4, 5, 7, 9, 11}},
      {"aeolian", {0, 2, 3, 5, 7, 8, 10}}
      //Turkish ??
  };

  unordered_map<string, vector<int>> arpegios = {
      {"major", {0, 4, 7}},
      {"minor", {0, 3, 7}},
      {"diminished", {0, 3, 6}},
      {"augmented", {0, 4, 8}},
      {"suspended", {0, 5, 7}},
      {"dominant", {0, 4, 7, 10}},
      {"major7", {0, 4, 7, 11}},
      {"minor7", {0, 3, 7, 10}},
      {"diminished7", {0, 3, 6, 9}},
      {"augmented7", {0, 4, 8, 10}},
      {"suspended7", {0, 5, 7, 10}},
      {"dominant7", {0, 4, 7, 10}},
      {"major9", {0, 4, 7, 11, 14}},
      {"minor9", {0, 3, 7, 10, 14}},
      {"diminished9", {0, 3, 6, 9, 13}},
      {"augmented9", {0, 4, 8, 10, 14}}};

  unordered_map<string, vector<int>> intervals = {
      {"second", {0, 2}},    {"third", {0, 4}},    {"fourth", {0, 5}},
      {"fifth", {0, 7}},     {"sixth", {0, 9}},    {"seventh", {0, 11}},
      {"octave", {0, 12}},   {"ninth", {0, 14}},   {"tenth", {0, 16}},
      {"eleventh", {0, 17}}, {"twelveth", {0, 19}}};

 private:
  vector<int> generate_full_scale(int rootNote, int nb_notes, string pattern,
                                  string scaleType);
  vector<int> generate_base_scale(int rootNote, string pattern,
                                  string scaleType);
};

#endif  //MIDI_TRANSLATOR_H