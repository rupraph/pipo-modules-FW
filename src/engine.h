#ifndef ENGINE_H
#define ENGINE_H

#include <Arduino.h>
#include <unordered_map>
#include "midi/midi_translator.h"
#include "hid/usb_hid.h"
#include "hid/hid_translator.h"
#include "midi/midi_io.h"
#include "utils/json.hpp"
#include "utils/fs_tools.h"
#include "sensor/input_sensor.h"
#include "hw_ui.h"
#include "osc_handler.h"
#include "osc_translators.h"

using namespace std;

// this class takes care of the translation of the sensor data to the different outputs
// Todo: maps should not be declared per axis, but per function since some output might rely on 2 inputs

class OSC_handler; // why do I need forward declaration here??

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
                    {"roll",OscTranslator()},
                    {"pitch",OscTranslator()},
                    {"yaw",OscTranslator()},
                    {"accX",OscTranslator()},
                    {"accY",OscTranslator()},
                    {"accZ",OscTranslator()}
                };


                HID_translators ={
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
                    {"dist",OscTranslator()}
                };
                HID_translators ={
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
                    {"A1",OscTranslator()},
                    {"A2",OscTranslator()},
                    {"A3",OscTranslator()},
                    {"A4",OscTranslator()},
                    {"A5",OscTranslator()},
                    {"A6",OscTranslator()},
                    {"T1",OscTranslator()},
                    {"T2",OscTranslator()},
                    {"T3",OscTranslator()},
                    {"T4",OscTranslator()},
                    {"T5",OscTranslator()},
                    {"T6",OscTranslator()}
                };

                HID_translators ={
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
        unordered_map<string, HidTranslator> HID_translators;

        hid_gamepad_report_t    gp;
        hid_keyboard_report_t    kb;
        hid_mouse_report_t       mouse;


        void update(Sensor& sensor, midi_io& midiio,usb_hid& hidio,OSC_handler& osc);

        void midi_processor(Sensor& sensor,midi_io& midiio);
        void hid_processor(Sensor& sensor,usb_hid& hidio);
        void osc_processor(Sensor& sensor,OSC_handler& osc);

        json get_config(bool debug=false);
        void set_config(json& config,bool debug=false);
        friend void to_json(json& j, const Engine& t);

    private:
        uint8_t note_val[128];
        uint8_t note_val_prev[128];


};


#endif //ENGINE_H