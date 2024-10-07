#ifndef MIDI_TRANSLATOR_H
#define MIDI_TRANSLATOR_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <MIDI.h>
#include "utils/json.hpp"

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

  int channel = 1;  // should be in engine.
  int cc_nb = 1;

  bool enabled = false;

  //notes variables
  int tl_mode = 0;  //0=cc, 1 note, 2 both

  string scaleType = "major";
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
  void printScale(vector<int> scale);
  void set_Scale_Type(string scaleType);
  void set_root_note(string rootNote);
  void set_number_of_notes(int nbOfNotes);
  int convertNoteNameToNumber(string noteName);
  string convertNumberToNoteName(int noteNumber);
  bool is_a_note(string noteName);
  vector<string> get_scale_names();
  void update_scale();

  //cc methods
  int get_cc_val(float value, float min_input, float max_input,
                 bool hires = false);
  int map_linear(float x, float min_input, float max_input);

  // save/load
  friend void to_json(nlohmann::json& j, const MidiTranslator& t);
  friend void from_json(const nlohmann::json& j, MidiTranslator& t);
  nlohmann::json get_json() const;
  void set_from_json(const nlohmann::json& j);
  string serialize() const;
  void deserialize(const string& data);

  // Getter setters
  bool getHires() const;
  void setHires(bool h);
  int getChannel();
  void setChannel(int c);
  int getCcNumber();
  void setCcNumber(int c);
  int getTranslatorMode();
  void setTranslatorMode(int t);
  string getScaleType();
  void setScaleType(string s);
  int getRootNote();
  void setRootNote(int r);
  int getNumberOfNotes();
  void setNumberOfNotes(int n);
  float getSustain();
  void setSustain(float s);
  int getMaxOutput();
  void setMaxOutput(int m);
  int getMinOutput();
  void setMinOutput(int m);
  int getInterpolationType();
  void setInterpolationType(int i);
  bool getEnabled();
  void setEnabled(bool e);

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

 private:
  vector<int> generate_full_Scale(int rootNote, int nb_notes, string scaleType);
  vector<int> generate_base_Scale(int rootNote, string scaleType);
};

#endif  //MIDI_TRANSLATOR_H