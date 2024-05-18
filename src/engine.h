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
        Engine(Sensor& sensor)
        {
             #if defined(PIPO_MOTION)
                Miditranslators ={
                    {"roll",MidiTranslator(sensor.get_limit_max("roll"))},
                    {"pitch",MidiTranslator(sensor.get_limit_max("pitch"))},
                    {"yaw",MidiTranslator(sensor.get_limit_max("yaw"))},
                    {"accX",MidiTranslator(sensor.get_limit_max("accX"))},
                    {"accY",MidiTranslator(sensor.get_limit_max("accY"))},
                    {"accZ",MidiTranslator(sensor.get_limit_max("accZ"))}   

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
                    {"dist",MidiTranslator(sensor.get_limit_max("dist"))}
                };
                hid_map ={
                    {"dist",HidTranslator()}
                };
            #elif defined(PIPO_ANALOG)
                Miditranslators ={
                    {"A1",MidiTranslator(sensor.get_limit_max("A1"))},
                    {"A2",MidiTranslator(sensor.get_limit_max("A2"))},
                    {"A3",MidiTranslator(sensor.get_limit_max("A3"))},
                    {"A4",MidiTranslator(sensor.get_limit_max("A4"))},
                    {"A5",MidiTranslator(sensor.get_limit_max("A5"))},
                    {"A6",MidiTranslator(sensor.get_limit_max("A6"))},
                    {"T1",MidiTranslator(sensor.get_limit_max("T1"))},
                    {"T2",MidiTranslator(sensor.get_limit_max("T2"))},
                    {"T3",MidiTranslator(sensor.get_limit_max("T3"))},
                    {"T4",MidiTranslator(sensor.get_limit_max("T4"))},
                    {"T5",MidiTranslator(sensor.get_limit_max("T5"))},
                    {"T6",MidiTranslator(sensor.get_limit_max("T6"))}
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