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

  int velocity = 100;  // default velocity for notes

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

  bool is_enabled();
  void set_enabled(bool e);

  int get_velocity();
  void set_velocity(int v);

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

 private:
  vector<int> generate_full_scale(int rootNote, int nb_notes, string pattern,
                                  string scaleType);
  vector<int> generate_base_scale(int rootNote, string pattern,
                                  string scaleType);

  static const unordered_map<string, vector<int>> arpegios;
  static const unordered_map<string, vector<int>> scales;
  static const unordered_map<string, vector<int>> intervals;
};

#endif  //MIDI_TRANSLATOR_H