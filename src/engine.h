#ifndef ENGINE_H
#define ENGINE_H

#include <Arduino.h>
#include <unordered_map>
#include "midi/midi_translator.h"
#include "hid/hid_translator.h"
#include "midi/midi_io.h"
#include "hid/usb_hid.h"
#include "utils/json.hpp"
#include "utils/fs_tools.h"
#include "sensor/input_sensor.h"
#include "hw_ui.h"
#include "osc_handler.h"
#include "osc_translators.h"

using namespace std;

// this class will process each data stream from the sensor, and depending on the applied settings, process it further, to finally send it on the selected output interface

// maps should not be declare per axis, but per function since some output might rely on 2 inputs

class OSC_handler; // why do I need forward declaration here??
//class OSC_translator;

class Engine
{
    public:
        Engine(Sensor& sensor)
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

                Osctranslators ={
                    {"roll",OscTranslator(sensor.get_limit_min("roll"),sensor.get_limit_max("roll"))},
                    {"pitch",OscTranslator(sensor.get_limit_min("pitch"),sensor.get_limit_max("pitch"))},
                    {"yaw",OscTranslator(sensor.get_limit_min("yaw"),sensor.get_limit_max("yaw"))},
                    {"accX",OscTranslator(sensor.get_limit_min("accX"),sensor.get_limit_max("accX"))},
                    {"accY",OscTranslator(sensor.get_limit_min("accY"),sensor.get_limit_max("accY"))},
                    {"accZ",OscTranslator(sensor.get_limit_min("accZ"),sensor.get_limit_max("accZ"))}
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
                Osctranslators ={
                    {"dist",OscTranslator(sensor.get_limit_min("dist"),sensor.get_limit_max("dist"))}
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

                Osctranslators ={
                    {"A1",OscTranslator(sensor.get_limit_min("A1"),sensor.get_limit_max("A1"))},
                    {"A2",OscTranslator(sensor.get_limit_min("A2"),sensor.get_limit_max("A2"))},
                    {"A3",OscTranslator(sensor.get_limit_min("A3"),sensor.get_limit_max("A3"))},
                    {"A4",OscTranslator(sensor.get_limit_min("A4"),sensor.get_limit_max("A4"))},
                    {"A5",OscTranslator(sensor.get_limit_min("A5"),sensor.get_limit_max("A5"))},
                    {"A6",OscTranslator(sensor.get_limit_min("A6"),sensor.get_limit_max("A6"))},
                    {"T1",OscTranslator(sensor.get_limit_min("T1"),sensor.get_limit_max("T1"))},
                    {"T2",OscTranslator(sensor.get_limit_min("T2"),sensor.get_limit_max("T2"))},
                    {"T3",OscTranslator(sensor.get_limit_min("T3"),sensor.get_limit_max("T3"))},
                    {"T4",OscTranslator(sensor.get_limit_min("T4"),sensor.get_limit_max("T4"))},
                    {"T5",OscTranslator(sensor.get_limit_min("T5"),sensor.get_limit_max("T5"))},
                    {"T6",OscTranslator(sensor.get_limit_min("T6"),sensor.get_limit_max("T6"))}
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
        unordered_map<string, OscTranslator> Osctranslators;
        unordered_map<string, HidTranslator> hid_map;

        hid_gamepad_report_t    gp;
        hid_keyboard_report_t    kb;
        hid_mouse_report_t       mouse;


        void update(Sensor& sensor, midi_io& midiio,usb_hid& hidio,OSC_handler& osc);

        void midi_processor(Sensor& sensor,midi_io& midiio);
        void hid_processor(Sensor& sensor,usb_hid& hidio);
        void osc_processor(Sensor& sensor,OSC_handler& osc);

        void set_default_config();
        json get_config(bool debug=false);
        void set_config(json& config,bool debug=false);
        friend void to_json(json& j, const Engine& t);

    private:
        uint8_t note_val[128];
        uint8_t note_val_prev[128];


};


#endif //ENGINE_H