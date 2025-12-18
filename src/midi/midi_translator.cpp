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
    log_e("Error: Invalid input range");
    return 0;  // or handle the error as needed
  }

  // Check if current_scale is empty (can happen with invalid config)
  if (current_scale.empty()) {
    log_e("Error: current_scale is empty, cannot get note");
    return rootNote;  // Return root note as safe fallback
  }

  // scale value from 0 to 1 to the range of the current scale
  // map value from input range to 0-1
  float scaled_value = (value - min_input) / (max_input - min_input);
  int index = round(scaled_value * (nbOfNotes - 1));
  index = constrain(index, 0, nbOfNotes - 1);

  // Additional safety check for index bounds
  if (index >= current_scale.size()) {
    log_e("Error: index %d out of bounds for scale size %d", index,
          current_scale.size());
    index = current_scale.size() - 1;
  }

  return current_scale[index];
}

void MidiTranslator::print_scale(vector<uint8_t> scale) {
  log_d("Scale: ");
  for (int i = 0; i < scale.size(); i++) {
    log_d("%d-", scale[i]);
  }
  log_d("");
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

void MidiTranslator::set_number_of_notes(uint8_t nbOfNotes) {
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

vector<uint8_t> MidiTranslator::generate_full_scale(int rootNote, int nb_notes,
                                                    string pattern,
                                                    string scaleType) {
  vector<uint8_t> expandedScale;
  // expand over octaves for arpeggios and scales
  if (pattern != "interval") {
    vector<uint8_t> scale = generate_base_scale(rootNote, pattern, scaleType);

    // Check if scale generation failed (empty vector)
    if (scale.empty()) {
      log_e("Failed to generate scale - using default major scale");
      // Fallback to major scale to prevent crash
      scale = generate_base_scale(rootNote, "scale", "major");
      if (scale.empty()) {
        // Last resort: return chromatic scale starting from rootNote
        log_e("Critical: default scale failed, using chromatic fallback");
        for (int i = 0; i < nb_notes && (rootNote + i) <= 127; i++) {
          expandedScale.push_back(rootNote + i);
        }
        return expandedScale;
      }
    }

    int baseScaleSize = scale.size();
    int baseNoteIndex = 0;
    int expandedNote = scale[baseNoteIndex];

    for (int i = 0; i < nb_notes; i++) {
      expandedScale.push_back(expandedNote);
      baseNoteIndex = (baseNoteIndex + 1) % baseScaleSize;
      expandedNote = scale[baseNoteIndex] + ((i + 1) / baseScaleSize) * 12;
    }
  } else {
    // build intervals
    int expandedNote = rootNote;

    // Validate interval type exists
    if (intervals.find(scaleType) == intervals.end()) {
      log_e("Invalid interval type '%s' - using octave interval as fallback",
            scaleType.c_str());
      scaleType = "octave";  // Safe fallback
    }

    int intervalStep = intervals.at(scaleType)[1];

    for (int i = 0; i < nb_notes; i++) {
      expandedScale.push_back(expandedNote);
      expandedNote += intervalStep;

      // Prevent MIDI note overflow
      if (expandedNote > 127) {
        log_w("Interval exceeded MIDI range, capping at note %d",
              expandedScale.back());
        break;
      }

      log_d("Note: %d", expandedNote);
      log_d("Interval: ");
      for (int j = 0; j < expandedScale.size(); j++) {
        log_d("%d-", expandedScale[j]);
      }
      log_d("");
    }
  }
  return expandedScale;
}

vector<uint8_t> MidiTranslator::generate_base_scale(int rootNote,
                                                    string pattern,
                                                    string scaleType) {
  if (pattern == "scale") {
    auto it = scales.find(scaleType);
    if (it != scales.end()) {
      vector<uint8_t> scale(it->second.begin(), it->second.end());
      for (int i = 0; i < scale.size(); i++) {
        scale[i] += rootNote;
      }
      return scale;
    } else {
      log_w("Invalid scale type");
      return {};
    }
  } else if (pattern == "arpeggio") {
    auto it = arpegios.find(scaleType);
    if (it != arpegios.end()) {
      vector<uint8_t> scale(it->second.begin(), it->second.end());
      for (int i = 0; i < scale.size(); i++) {
        scale[i] += rootNote;
      }
      return scale;
    } else {
      log_w("Invalid arpeggio type");
      return {};
    }
  } else {
    log_w("Invalid pattern type");
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
  j["velocity"] = velocity;  // Assuming velocity is a member variable
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
    velocity = j["velocity"];  // Assuming velocity is a member variable

    // Update scale and verify it succeeded
    this->update_scale();

    if (current_scale.empty()) {
      log_e(
          "Failed to generate valid scale from config - pattern: %s, "
          "scaleType: %s",
          pattern.c_str(), scaleType.c_str());
      // Force safe defaults
      pattern = "scale";
      scaleType = "major";
      this->update_scale();
    }

  } catch (std::exception& e) {
    log_e("MIDI translator error: %s", e.what());
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
uint8_t MidiTranslator::get_channel() {
  return channel;
}
void MidiTranslator::set_channel(uint8_t c) {
  channel = c;
}

uint8_t MidiTranslator::get_cc_number() {
  return cc_nb;
}
void MidiTranslator::set_cc_number(uint8_t c) {
  cc_nb = c;
}

uint8_t MidiTranslator::get_translator_mode() {
  return tl_mode;
}
void MidiTranslator::set_translator_mode(uint8_t t) {
  tl_mode = t;
}

string MidiTranslator::get_scale_type() {
  return scaleType;
}

uint8_t MidiTranslator::get_root_note() {
  return rootNote;
}
void MidiTranslator::set_root_note(uint8_t r) {
  rootNote = r;
}

uint8_t MidiTranslator::get_number_of_notes() {
  return nbOfNotes;
}

float MidiTranslator::get_sustain() {
  return sustain;
}
void MidiTranslator::set_sustain(float s) {
  sustain = s;
}

uint16_t MidiTranslator::get_max_output() {
  return cc_max;
}
void MidiTranslator::set_max_output(uint16_t m) {
  cc_max = m;
}

uint16_t MidiTranslator::get_min_output() {
  return cc_min;
}
void MidiTranslator::set_min_output(uint16_t m) {
  cc_min = m;
}

uint8_t MidiTranslator::get_velocity() {
  return velocity;
}

void MidiTranslator::set_velocity(uint8_t v) {
  velocity = v;
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

// Notes scale variables
// Todo: change architecture and add chords
// the format is inherited from a moment where we would put custom scales in these lists. now frozen, should be constant with smaller data types.
const unordered_map<string, vector<int>> MidiTranslator::scales = {
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

const unordered_map<string, vector<int>> MidiTranslator::arpegios = {
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

const unordered_map<string, vector<int>> MidiTranslator::intervals = {
    {"second", {0, 2}},    {"third", {0, 4}},    {"fourth", {0, 5}},
    {"fifth", {0, 7}},     {"sixth", {0, 9}},    {"seventh", {0, 11}},
    {"octave", {0, 12}},   {"ninth", {0, 14}},   {"tenth", {0, 16}},
    {"eleventh", {0, 17}}, {"twelveth", {0, 19}}};