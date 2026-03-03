#ifndef MIDI_TRANSLATOR_H
#define MIDI_TRANSLATOR_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <ArduinoJson.h>

using namespace std;

//This class is used to convert sensor value to usable midi messages

class MidiTranslator

// Todo:
// move all members to private and write methods to public

{
 public:
  MidiTranslator();
  bool enabled = false;
  uint8_t channel = 1;  // should be in engine.
  uint8_t cc_nb = 1;

  //notes variables
  uint8_t tl_mode = 0;  //0=cc, 1 note, 2 both

  string pattern = "scale";  //can be scale, arpegio or interval (note interval
  string scaleType = "major";  // see maps below for keywords
  uint8_t rootNote = 45;
  uint8_t nbOfNotes = 25;
  vector<uint8_t> current_scale;
  float sustain = 1.0;  //in seconds. 0 for infinite

  //CC variables
  uint16_t cc_max;
  uint16_t cc_min = 0;
  // int interpolation_type = 0;  //0=linear, 1=step, 2=log
  bool hires = false;

  uint8_t velocity = 100;  // default velocity for notes

  // Notes scale methods
  int get_note(float value, float min_input, float max_input);
  void print_scale(vector<uint8_t> scale);

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

  uint8_t get_channel();
  void set_channel(uint8_t c);

  uint8_t get_cc_number();
  void set_cc_number(uint8_t c);

  uint8_t get_translator_mode();
  void set_translator_mode(uint8_t t);

  string get_scale_type();
  void set_scale_type(string scaleType);

  uint8_t get_root_note();
  void set_root_note(string rootNote);
  void set_root_note(uint8_t notenb);

  uint8_t get_number_of_notes();
  void set_number_of_notes(uint8_t nbOfNotes);

  float get_sustain();
  void set_sustain(float s);

  uint16_t get_max_output();
  void set_max_output(uint16_t m);

  uint16_t get_min_output();
  void set_min_output(uint16_t m);

  int get_interpolation_type();
  void set_interpolation_type(int i);

  bool is_enabled();
  void set_enabled(bool e);

  uint8_t get_velocity();
  void set_velocity(uint8_t v);

 private:
  vector<uint8_t> generate_full_scale(int rootNote, int nb_notes,
                                      string pattern, string scaleType);
  vector<uint8_t> generate_base_scale(int rootNote, string pattern,
                                      string scaleType);

  static const unordered_map<string, vector<int>> arpegios;
  static const unordered_map<string, vector<int>> scales;
  static const unordered_map<string, vector<int>> intervals;
};

#endif  //MIDI_TRANSLATOR_H