#ifndef ENGINE_H
#define ENGINE_H

#include "acc_sensor.h"
#include <unordered_map>
#include "midi_translator.h"
#include "hid_translator.h"
#include "midi_io.h"
#include "usb_hid.h"
#include "json.hpp"
#include "fs_tools.h"

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
                    {"range",MidiTranslator()}
                };
                hid_map ={
                    {"range",HidTranslator()}
                };
            #elif defined(PIPO_ANALOG)
                Miditranslators ={
                    {"analog1",MidiTranslator()},
                    {"analog2",MidiTranslator()},
                    {"analog3",MidiTranslator()},
                    {"analog4",MidiTranslator()}
                };
                hid_map ={
                    {"analog",HidTranslator()},
                    {"analog2",HidTranslator()},
                    {"analog3",HidTranslator()},
                    {"analog4",HidTranslator()}
                };
            #endif
        }
       
        unordered_map<string, MidiTranslator> Miditranslators;
        unordered_map<string, HidTranslator> hid_map;

        hid_gamepad_report_t    gp;
        hid_keyboard_report_t    kb;
        hid_mouse_report_t       mouse;


        void update(midi_io& midiio,usb_hid& hidio);
        void midi_processsor(midi_io& midiio);
        void hid_processor(usb_hid& hidio);
        void set_default_config();
        json get_config(bool debug=false);
        void set_config(json& config,bool debug=false);
        friend void to_json(json& j, const Engine& t);

};


#endif //ENGINE_H