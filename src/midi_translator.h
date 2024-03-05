#ifndef MIDI_TRANSLATOR_H
#define MIDI_TRANSLATOR_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <MIDI.h>

using namespace std;

class MidiTranslator 
{
    public:
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
        };

        string scaleType="major";
        int rootNote = 60;
        int minimum_note = 45;
        int maximumNote = 69;

        vector<int> current_scale;

        //int first_note = 40;
        //int total_note_number = 15;
        //string mode = "with_start_and_number";


        MidiTranslator();
        int get_note(float value);
        void printScale(vector<int> scale);
        void setScale(string scaleType);
        void set_root_note(string rootNote);
        //void set_with_start_and_number(string first_note, int total_note_number);
        void set_with_min_and_max(string first_note, string last_note);

        int convertNoteNameToNumber(string noteName);
        string convertNumberToNoteName(int noteNumber); 

        bool is_a_note(string noteName);

        vector<string> get_scale_names();

    private:
        vector<int> generate_full_Scale(int minNote, int maxNote, string scaleType, int rootNote);
        vector<int> generate_base_Scale(int rootNote, string scaleType);

};

#endif //MIDI_TRANSLATOR_H