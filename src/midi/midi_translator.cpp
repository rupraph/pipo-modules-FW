#include "midi_translator.h"

MidiTranslator::MidiTranslator() {
  current_scale.clear();
  current_scale = generate_full_scale(rootNote, nbOfNotes, pattern, scaleType);
  if (hires) {
    cc_max = 16383;
  } else {
    cc_max = 127;
  }
}

int MidiTranslator::get_note(float value, float min_input, float max_input) {

  float input_range = max_input - min_input;
  if (input_range == 0) {
    Serial.println("Error: Invalid input range");
    return 0;  // or handle the error as needed
  }

  // scale value from 0 to 1 to the range of the current scale
  // map value from input range to 0-1
  float scaled_value = (value - min_input) / (max_input - min_input);
  int index = round(scaled_value * (nbOfNotes - 1));
  index = constrain(index, 0, nbOfNotes - 1);
  return current_scale[index];
}

void MidiTranslator::print_scale(vector<int> scale) {
  Serial.println("Scale: ");
  for (int i = 0; i < scale.size(); i++) {
    Serial.print(scale[i]);
    Serial.print("-");
  }
  Serial.println();
}

void MidiTranslator::set_scale_type(string scaleType) {
  auto it = scales.find(scaleType);
  if (it != scales.end()) {
    scaleType = scaleType;
  } else {
    cout << "Invalid scale type." << endl;
  }
  current_scale.clear();
  current_scale = generate_full_scale(rootNote, nbOfNotes, pattern, scaleType);
}

void MidiTranslator::set_root_note(string rootNote) {
  int rootNotenb = convert_note_name_to_number(rootNote);
  if (rootNotenb < 0 || rootNotenb > 127) {
    cout << "Invalid root note." << endl;
    return;
  } else {
    rootNotenb = rootNotenb;
    current_scale.clear();
    current_scale =
        generate_full_scale(rootNotenb, nbOfNotes, pattern, scaleType);
  }
}

void MidiTranslator::set_number_of_notes(int nbOfNotes) {
  if (nbOfNotes < 0 || nbOfNotes > 127) {
    cout << "Invalid number of notes." << endl;
    return;
  } else {
    this->nbOfNotes = nbOfNotes;
    current_scale.clear();
    current_scale =
        generate_full_scale(rootNote, nbOfNotes, pattern, scaleType);
  }
}

int MidiTranslator::convert_note_name_to_number(string noteName) {
  unordered_map<string, int> noteMap = {
      {"C", 0},  {"C#", 1}, {"Db", 1},  {"D", 2},   {"D#", 3}, {"Eb", 3},
      {"E", 4},  {"F", 5},  {"F#", 6},  {"Gb", 6},  {"G", 7},  {"G#", 8},
      {"Ab", 8}, {"A", 9},  {"A#", 10}, {"Bb", 10}, {"B", 11}};

  string note = noteName.substr(0, noteName.length() - 1);
  int octave = stoi(noteName.substr(noteName.length() - 1));

  return noteMap[note] + (octave + 1) * 12;
}

string MidiTranslator::convert_number_to_note_name(int noteNumber) {
  unordered_map<int, string> noteMap = {
      {0, "C"},  {1, "C#"}, {2, "D"},  {3, "D#"}, {4, "E"},   {5, "F"},
      {6, "F#"}, {7, "G"},  {8, "G#"}, {9, "A"},  {10, "A#"}, {11, "B"}};

  int octave = noteNumber / 12 - 1;
  int note = noteNumber % 12;

  return noteMap[note] + to_string(octave);
}

bool MidiTranslator::is_a_note(string noteName) {
  int note = convert_note_name_to_number(noteName);
  if (note < 0 || note > 127) {
    return false;
  } else {
    return true;
  }
}

vector<int> MidiTranslator::generate_full_scale(int rootNote, int nb_notes,
                                                string pattern,
                                                string scaleType) {

  vector<int> scale = generate_base_scale(rootNote, pattern, scaleType);
  vector<int> expandedScale;

  int baseScaleSize = scale.size();
  int baseNoteIndex = 0;
  int expandedNote = scale[baseNoteIndex];

  for (int i = 0; i < nb_notes; i++) {
    expandedScale.push_back(expandedNote);
    baseNoteIndex = (baseNoteIndex + 1) % baseScaleSize;
    expandedNote = scale[baseNoteIndex] + ((i + 1) / baseScaleSize) * 12;
  }

  return expandedScale;
}

vector<int> MidiTranslator::generate_base_scale(int rootNote, string pattern,
                                                string scaleType) {
  if (pattern == "scale") {
    auto it = scales.find(scaleType);
    if (it != scales.end()) {
      vector<int> scale = it->second;
      for (int i = 0; i < scale.size(); i++) {
        scale[i] += rootNote;
      }
      return scale;
    } else {
      Serial.println("Invalid scale type");
      return {};
    }
  } else if (pattern == "arpeggio") {
    auto it = arpegios.find(scaleType);
    if (it != arpegios.end()) {
      vector<int> scale = it->second;
      for (int i = 0; i < scale.size(); i++) {
        scale[i] += rootNote;
      }
      return scale;
    } else {
      Serial.println("Invalid arpeggio type");
      return {};
    }
  } else if (pattern == "interval") {
    auto it = intervals.find(scaleType);
    if (it != intervals.end()) {
      vector<int> scale = it->second;
      for (int i = 0; i < scale.size(); i++) {
        scale[i] += rootNote;
      }
      return scale;
    } else {
      Serial.println("Invalid interval type");
      return {};
    }
  } else {
    Serial.println("Invalid pattern type");
    return {};
  }
}

vector<string> MidiTranslator::get_scale_names() {
  vector<string> scale_names;
  for (auto it : scales) {
    scale_names.push_back(it.first);
  }
  return scale_names;
}

void MidiTranslator::update_scale() {
  current_scale.clear();
  current_scale = generate_full_scale(this->rootNote, this->nbOfNotes,
                                      this->pattern, this->scaleType);
}

int MidiTranslator::get_cc_val(float value, float min_input, float max_input,
                               bool hires = false) {
  // this returns a scaled value from the input range (max_input/min_input) to
  // the output range (max_output/min_output)

  if (hires) {
    cc_max = 16383;
  } else {
    cc_max = 127;
  }

  // if (interpolation_type == 0) {
  return map_linear(value, min_input, max_input);
  // } else {
  //   // not implemented yet
  //   return 0;
  // }
}

int MidiTranslator::map_linear(float value, float min_input, float max_input) {
  if (min_input == max_input || cc_min == cc_max) {
    Serial.println("min and max values cannot be equal");
  }
  return round((value - min_input) / (max_input - min_input) *
                   (cc_max - cc_min) +
               cc_min);
}

JsonDocument MidiTranslator::get_json() const {
  JsonDocument j;
  j["enabled"] = enabled;
  j["channel"] = channel;
  j["cc_nb"] = cc_nb;
  j["tl_mode"] = tl_mode;
  j["pattern"] = pattern;
  j["scaleType"] = scaleType;
  j["rootNote"] = rootNote;
  j["nbOfNotes"] = nbOfNotes;
  j["sustain"] = sustain;
  j["cc_max"] = cc_max;
  j["cc_min"] = cc_min;
  j["hires"] = hires;
  return j;
}

void MidiTranslator::set_from_json(const JsonDocument& j) {
  try {
    enabled = j["enabled"];
    channel = j["channel"];
    cc_nb = j["cc_nb"];
    tl_mode = j["tl_mode"];
    pattern = j["pattern"].as<string>();
    scaleType = j["scaleType"].as<string>();
    rootNote = j["rootNote"];
    nbOfNotes = j["nbOfNotes"];
    sustain = j["sustain"];
    cc_max = j["cc_max"];
    cc_min = j["cc_min"];
    hires = j["hires"];
    this->update_scale();

  } catch (const std::exception& e) {
    Serial.print("Error: ");
    Serial.println(e.what());
  }
}

//Getter setters

bool MidiTranslator::get_hires() const {
  return hires;
}

void MidiTranslator::set_hires(bool h) {
  hires = h;
  if (hires) {
    cc_max = 16383;
  } else {
    cc_max = 127;
  }
}

// Todo: should make setter more secure with value checking
int MidiTranslator::get_channel() {
  return channel;
}
void MidiTranslator::set_channel(int c) {
  channel = c;
}

int MidiTranslator::get_cc_number() {
  return cc_nb;
}
void MidiTranslator::set_cc_number(int c) {
  cc_nb = c;
}

int MidiTranslator::get_translator_mode() {
  return tl_mode;
}
void MidiTranslator::set_translator_mode(int t) {
  tl_mode = t;
}

string MidiTranslator::get_scale_type() {
  return scaleType;
}

int MidiTranslator::get_root_note() {
  return rootNote;
}
void MidiTranslator::set_root_note(int r) {
  rootNote = r;
}

int MidiTranslator::get_number_of_notes() {
  return nbOfNotes;
}

float MidiTranslator::get_sustain() {
  return sustain;
}
void MidiTranslator::set_sustain(float s) {
  sustain = s;
}

int MidiTranslator::get_max_output() {
  return cc_max;
}
void MidiTranslator::set_max_output(int m) {
  cc_max = m;
}

int MidiTranslator::get_min_output() {
  return cc_min;
}
void MidiTranslator::set_min_output(int m) {
  cc_min = m;
}

// int MidiTranslator::get_interpolation_type() {
//   return interpolation_type;
// }
// void MidiTranslator::set_interpolation_type(int i) {
//   interpolation_type = i;
// }

bool MidiTranslator::is_enabled() {
  return enabled;
}
void MidiTranslator::set_enabled(bool e) {
  enabled = e;
}
