#include "midi_translator.h"

using json = nlohmann::json;

MidiTranslator::MidiTranslator() {
  current_scale.clear();
  current_scale = generate_full_Scale(rootNote, numberOfNotes, scaleType);
  if (hires) {
    max_output = 16383;
  } else {
    max_output = 127;
  }
}

int MidiTranslator::get_note(float value, float min_input, float max_input) {
  //cap value to input range
  float out_value = value;
  if (value < min_input) {
    out_value = min_input + 1;
  } else if (value > max_input) {
    out_value = max_input;
  }

  float input_range = max_input - min_input;
  if (input_range == 0) {
    Serial.println("Error: Invalid input range");
    return 0;  // or handle the error as needed
  }

  // scale value from 0 to 1 to the range of the current scale
  // map value from input range to 0-1
  float scaledValue = (out_value - min_input) / (max_input - min_input);
  int index = round(scaledValue * (numberOfNotes - 1));
  index = constrain(index, 0, numberOfNotes - 1);
  return current_scale[index];
}

void MidiTranslator::printScale(vector<int> scale) {
  Serial.println("Scale: ");
  for (int i = 0; i < scale.size(); i++) {
    Serial.print(scale[i]);
    Serial.print("-");
  }
  Serial.println();
}

void MidiTranslator::set_Scale_Type(string scaleType) {
  auto it = scales.find(scaleType);
  if (it != scales.end()) {
    scaleType = scaleType;
  } else {
    cout << "Invalid scale type." << endl;
  }
  current_scale.clear();
  current_scale = generate_full_Scale(rootNote, numberOfNotes, scaleType);
}

void MidiTranslator::set_root_note(string rootNote) {
  int rootNotenb = convertNoteNameToNumber(rootNote);
  if (rootNotenb < 0 || rootNotenb > 127) {
    cout << "Invalid root note." << endl;
    return;
  } else {
    rootNotenb = rootNotenb;
    current_scale.clear();
    current_scale = generate_full_Scale(rootNotenb, numberOfNotes, scaleType);
  }
}

void MidiTranslator::set_number_of_notes(int numberOfNotes) {
  if (numberOfNotes < 0 || numberOfNotes > 127) {
    cout << "Invalid number of notes." << endl;
    return;
  } else {
    numberOfNotes = numberOfNotes;
    current_scale.clear();
    current_scale = generate_full_Scale(rootNote, numberOfNotes, scaleType);
  }
}

int MidiTranslator::convertNoteNameToNumber(string noteName) {
  unordered_map<string, int> noteMap = {
      {"C", 0},  {"C#", 1}, {"Db", 1},  {"D", 2},   {"D#", 3}, {"Eb", 3},
      {"E", 4},  {"F", 5},  {"F#", 6},  {"Gb", 6},  {"G", 7},  {"G#", 8},
      {"Ab", 8}, {"A", 9},  {"A#", 10}, {"Bb", 10}, {"B", 11}};

  string note = noteName.substr(0, noteName.length() - 1);
  int octave = stoi(noteName.substr(noteName.length() - 1));

  return noteMap[note] + (octave + 1) * 12;
}

string MidiTranslator::convertNumberToNoteName(int noteNumber) {
  unordered_map<int, string> noteMap = {
      {0, "C"},  {1, "C#"}, {2, "D"},  {3, "D#"}, {4, "E"},   {5, "F"},
      {6, "F#"}, {7, "G"},  {8, "G#"}, {9, "A"},  {10, "A#"}, {11, "B"}};

  int octave = noteNumber / 12 - 1;
  int note = noteNumber % 12;

  return noteMap[note] + to_string(octave);
}

bool MidiTranslator::is_a_note(string noteName) {
  int note = convertNoteNameToNumber(noteName);
  if (note < 0 || note > 127) {
    return false;
  } else {
    return true;
  }
}

vector<int> MidiTranslator::generate_full_Scale(int rootNote, int nb_notes,
                                                string scaleType) {

  vector<int> scale = generate_base_Scale(rootNote, scaleType);
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

vector<int> MidiTranslator::generate_base_Scale(int rootNote,
                                                string scaleType) {
  auto it = scales.find(scaleType);
  if (it != scales.end()) {
    vector<int> scale = it->second;
    for (int i = 0; i < scale.size(); i++) {
      scale[i] += rootNote;
    }
    return scale;
  } else {
    cout << "Invalid scale type." << endl;
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
  current_scale =
      generate_full_Scale(this->rootNote, this->numberOfNotes, this->scaleType);
}

int MidiTranslator::get_cc_val(float value, float min_input, float max_input,
                               bool hires = false) {
  // this returns a scaled value from the input range (max_input/min_input) to the output range (max_output/min_output)

  //cap value to input range
  if (value < min_input) {
    value = min_input;
  } else if (value > max_input) {
    value = max_input;
  }
  if (hires) {
    max_output = 16383;
  } else {
    max_output = 127;
  }

  if (interpolation_type == 0) {
    return map_linear(value, min_input, max_input);
  } else {
    // not implemented yet
    return 0;
  }
}

int MidiTranslator::map_linear(float value, float min_input, float max_input) {
  if (min_input == max_input || min_output == max_output) {
    Serial.println("min and max values cannot be equal");
  }
  return round((value - min_input) / (max_input - min_input) *
                   (max_output - min_output) +
               min_output);
}

void to_json(json& j, const MidiTranslator& t) {
  j = json{{"translator_mode", t.translator_mode},
           {"scaleType", t.scaleType},
           {"rootNote", t.rootNote},
           {"numberOfNotes", t.numberOfNotes},
           {"sustain", t.sustain},
           // {"current_scale", t.current_scale},
           // {"max_input", t.max_input},
           // {"min_input", t.min_input},
           {"max_output", t.max_output},
           {"min_output", t.min_output},
           {"interpolation_type", t.interpolation_type},
           {"hires", t.hires},
           {"channel", t.channel},
           {"cc_number", t.cc_number},
           {"enabled", t.enabled}};
}

string MidiTranslator::serialize() const {
  json j = *this;
  return j.dump();
}

json MidiTranslator::get_json() const {
  return json(*this);
}

void from_json(const json& j, MidiTranslator& t) {
  j.at("translator_mode").get_to(t.translator_mode);
  j.at("scaleType").get_to(t.scaleType);
  j.at("rootNote").get_to(t.rootNote);
  j.at("numberOfNotes").get_to(t.numberOfNotes);
  j.at("sustain").get_to(t.sustain);
  //j.at("current_scale").get_to(t.current_scale);
  // j.at("max_input").get_to(t.max_input);
  // j.at("min_input").get_to(t.min_input);
  j.at("max_output").get_to(t.max_output);
  j.at("min_output").get_to(t.min_output);
  j.at("interpolation_type").get_to(t.interpolation_type);
  j.at("hires").get_to(t.hires);
  j.at("channel").get_to(t.channel);
  j.at("cc_number").get_to(t.cc_number);
  j.at("enabled").get_to(t.enabled);
}

void MidiTranslator::deserialize(const string& data) {
  json j = json::parse(data);
  *this = j.get<MidiTranslator>();
}

void MidiTranslator::set_from_json(const json& j) {
  try {
    *this = j.get<MidiTranslator>();
  } catch (const std::exception& e) {
    Serial.print("Error: ");
    Serial.println(e.what());
  }

  this->update_scale();
}

//Getter setters

bool MidiTranslator::getHires() const {
  return hires;
}

void MidiTranslator::setHires(bool h) {
  hires = h;
  if (hires) {
    max_output = 16383;
  } else {
    max_output = 127;
  }
}

// Todo: should make setter more secure with value checking
int MidiTranslator::getChannel() {
  return channel;
}
void MidiTranslator::setChannel(int c) {
  channel = c;
}

int MidiTranslator::getCcNumber() {
  return cc_number;
}
void MidiTranslator::setCcNumber(int c) {
  cc_number = c;
}

int MidiTranslator::getTranslatorMode() {
  return translator_mode;
}
void MidiTranslator::setTranslatorMode(int t) {
  translator_mode = t;
}

string MidiTranslator::getScaleType() {
  return scaleType;
}
void MidiTranslator::setScaleType(string s) {
  scaleType = s;
}

int MidiTranslator::getRootNote() {
  return rootNote;
}
void MidiTranslator::setRootNote(int r) {
  rootNote = r;
}

int MidiTranslator::getNumberOfNotes() {
  return numberOfNotes;
}
void MidiTranslator::setNumberOfNotes(int n) {
  numberOfNotes = n;
}

float MidiTranslator::getSustain() {
  return sustain;
}
void MidiTranslator::setSustain(float s) {
  sustain = s;
}

int MidiTranslator::getMaxOutput() {
  return max_output;
}
void MidiTranslator::setMaxOutput(int m) {
  max_output = m;
}

int MidiTranslator::getMinOutput() {
  return min_output;
}
void MidiTranslator::setMinOutput(int m) {
  min_output = m;
}

int MidiTranslator::getInterpolationType() {
  return interpolation_type;
}
void MidiTranslator::setInterpolationType(int i) {
  interpolation_type = i;
}

bool MidiTranslator::getEnabled() {
  return enabled;
}
void MidiTranslator::setEnabled(bool e) {
  enabled = e;
}
