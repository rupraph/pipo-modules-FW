#ifndef ENGINE_H
#define ENGINE_H

#include <unordered_map>
#include "midi_translator.h"
#include "hid_translator.h"
#include "midi_io.h"
#include "usb_hid.h"
#include "json.hpp"
#include "fs_tools.h"
#include "input_sensor.h"

using namespace std;

// this class will process each data stream from the sensor, and depending on the applied settings, process it further, to finally send it on the selected output interface

// maps should not be declare per axis, but per function since some output might rely on 2 inputs

class Engine
{
    public:
        Engine()
        {
             #if defined(PIPO_MOTION)
                Miditranslators ={
                    {"roll",MidiTranslator()},
                    {"pitch",MidiTranslator()},
                    {"yaw",MidiTranslator()},
                    {"accX",MidiTranslator()},
                    {"accY",MidiTranslator()},
                    {"accZ",MidiTranslator()}
                };
                hid_map ={
                    {"roll",HidTranslator()},
                    {"pitch",HidTranslator()},
                    {"yaw",HidTranslator()},
                    {"accX",HidTranslator()},
                    {"accY",HidTranslator()},
                    {"accZ",HidTranslator()}
                };
            #elif defined(PIPO_RANGE)
                Miditranslators ={
                    {"dist",MidiTranslator()}
                };
                hid_map ={
                    {"dist",HidTranslator()}
                };
            #elif defined(PIPO_ANALOG)
                Miditranslators ={
                    {"A1",MidiTranslator()},
                    {"A2",MidiTranslator()},
                    {"A3",MidiTranslator()},
                    {"A4",MidiTranslator()},
                    {"A5",MidiTranslator()},
                    {"A6",MidiTranslator()},
                    {"T1",MidiTranslator()},
                    {"T2",MidiTranslator()},
                    {"T3",MidiTranslator()},
                    {"T4",MidiTranslator()},
                    {"T5",MidiTranslator()},
                    {"T6",MidiTranslator()}
                };
                hid_map ={
                    {"A1",HidTranslator()},
                    {"A2",HidTranslator()},
                    {"A3",HidTranslator()},
                    {"A4",HidTranslator()},
                    {"A5",HidTranslator()},
                    {"A6",HidTranslator()},
                    {"T1",HidTranslator()},
                    {"T2",HidTranslator()},
                    {"T3",HidTranslator()},
                    {"T4",HidTranslator()},
                    {"T5",HidTranslator()},
                    {"T6",HidTranslator()}
                };
            #endif
        }
       
        unordered_map<string, MidiTranslator> Miditranslators;
        unordered_map<string, HidTranslator> hid_map;

        hid_gamepad_report_t    gp;
        hid_keyboard_report_t    kb;
        hid_mouse_report_t       mouse;


        void update(Sensor& sensor, midi_io& midiio,usb_hid& hidio);
        void midi_processsor(Sensor& sensor,midi_io& midiio);
        void hid_processor(Sensor& sensor,usb_hid& hidio);
        void set_default_config();
        json get_config(bool debug=false);
        void set_config(json& config,bool debug=false);
        friend void to_json(json& j, const Engine& t);

};


#endif //ENGINE_H