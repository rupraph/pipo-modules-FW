#ifndef MIDI_TRANSLATOR_H
#define MIDI_TRANSLATOR_H

#include <Arduino.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <MIDI.h>
#include "json.hpp"

using namespace std;

class MidiTranslator 
{
    public:
        MidiTranslator();

        //notes variables
        int translator_mode = 0; //0=cc, 1 note, 2 both
        string scaleType="major";
        int rootNote = 45;
        int numberOfNotes = 25;
        vector<int> current_scale;

        // common for note and cc
        float max_input = 180;
        float min_input = -180;


        //CC variables
        
        int max_output = 127;
        int min_output = 0;
        bool cc_resolution = 0; //0=7bit, 1=14bit
        int interpolation_type = 0; //0=linear, 1=step, 2=log

        // Notes scale variables

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



        int get_note(float value);
        void printScale(vector<int> scale);
        void set_Scale_Type(string scaleType);
        void set_root_note(string rootNote);
        void set_number_of_notes(int numberOfNotes);
        //void set_with_start_and_number(string first_note, int total_note_number);

        void set_every_note(vector<string> scale);
        void set_new_scale(string newscaleType,vector<string> newscale);

        int convertNoteNameToNumber(string noteName);
        string convertNumberToNoteName(int noteNumber); 

        bool is_a_note(string noteName);

        vector<string> get_scale_names();

        void update_scale();

        // To cc variables
        int get_cc_val(float value, bool hires=false);
        int map_linear(float x);


        // save/load
        void to_json(nlohmann::json& j,const MidiTranslator& t);
        string serialize() const;
        void from_json(const nlohmann::json& j, MidiTranslator& t);
        void deserialize(const string& data);


    private:
        vector<int> generate_full_Scale(int rootNote,int nb_notes, string scaleType);
        vector<int> generate_base_Scale(int rootNote, string scaleType);

};

#endif //MIDI_TRANSLATOR_H