#include "midi_translator.h"


MidiTranslator::MidiTranslator() {
    current_scale = generate_full_Scale(minimum_note, maximumNote, scaleType, rootNote);
    printScale(current_scale);
}

int MidiTranslator::get_note(float value) {
    // scale value from 0 to 1 to the range of the current scale
    float scaledValue = value * (current_scale.size() - 1);
    int index = round(scaledValue);
    return current_scale[index];
}

void MidiTranslator::printScale(vector<int> scale) {
    Serial.println("Scale: ");
    for (int i = 0; i < scale.size(); i++) {
        Serial.print(scale[i]);
        Serial.print("-");
    }
}

void MidiTranslator::setScale(string scaleType) {
    auto it = scales.find(scaleType);
    if (it != scales.end()) {
        scaleType = scaleType;
    } else {
        cout << "Invalid scale type." << endl;
    }

    current_scale = generate_full_Scale(minimum_note, maximumNote, scaleType, rootNote);
}

void MidiTranslator::set_root_note(string rootNote) {
    int rootNotenb = convertNoteNameToNumber(rootNote);
    if (rootNotenb < 0 || rootNotenb > 127) {
        cout << "Invalid root note." << endl;
        return;
    }
    else {
        rootNotenb = rootNotenb;
        current_scale = generate_full_Scale(minimum_note, maximumNote, scaleType, rootNotenb);
    }
}

void MidiTranslator::set_with_min_and_max(string first_note, string last_note) {
    int firstNotenb = convertNoteNameToNumber(first_note);
    int lastNotenb = convertNoteNameToNumber(last_note);
    if (firstNotenb < 0 || firstNotenb > 127 || lastNotenb < 0 || lastNotenb > 127) {
        cout << "Invalid note range." << endl;
        return;
    }
    else {
        firstNotenb = firstNotenb;
        lastNotenb = lastNotenb;
        current_scale = generate_full_Scale(firstNotenb, lastNotenb, scaleType, rootNote);
    }
}

int MidiTranslator::convertNoteNameToNumber(string noteName) {
    unordered_map<string, int> noteMap = {
        {"C", 0}, {"C#", 1}, {"Db", 1}, {"D", 2}, {"D#", 3}, {"Eb", 3},
        {"E", 4}, {"F", 5}, {"F#", 6}, {"Gb", 6}, {"G", 7}, {"G#", 8},
        {"Ab", 8}, {"A", 9}, {"A#", 10}, {"Bb", 10}, {"B", 11}
    };

    string note = noteName.substr(0, noteName.length() - 1);
    int octave = stoi(noteName.substr(noteName.length() - 1));

    return noteMap[note] + (octave + 1) * 12;
}

string MidiTranslator::convertNumberToNoteName(int noteNumber) {
    unordered_map<int, string> noteMap = {
        {0, "C"}, {1, "C#"}, {2, "D"}, {3, "D#"}, {4, "E"}, {5, "F"},
        {6, "F#"}, {7, "G"}, {8, "G#"}, {9, "A"}, {10, "A#"}, {11, "B"}
    };

    int octave = noteNumber / 12 - 1;
    int note = noteNumber % 12;

    return noteMap[note] + to_string(octave);
}

bool MidiTranslator::is_a_note(string noteName) {
    int note = convertNoteNameToNumber(noteName);
    if (note < 0 || note > 127) {
        return false;
    }
    else {
        return true;
    }
}

vector<int> MidiTranslator::generate_full_Scale(int minNote, int maxNote, string scaleType, int rootNote) {
    //int minNote = convertNoteNameToNumber(minimum_note);
    //int maxNote = convertNoteNameToNumber(maximumNote);

    vector<int> scale = generate_base_Scale(rootNote, scaleType);
    vector<int> expandedScale;

    int baseScaleSize = scale.size();

    for (int i = minNote; i <= maxNote; i++) {
        int baseNoteIndex = (i - minNote) % baseScaleSize;
        int expandedNote = scale[baseNoteIndex] + (i / baseScaleSize) * 12;
        expandedScale.push_back(expandedNote);
    }

    return expandedScale;
}



vector<int> MidiTranslator::generate_base_Scale(int rootNote, string scaleType) {
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

 


