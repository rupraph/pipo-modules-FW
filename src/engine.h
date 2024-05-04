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
#include "config.h"

using namespace std;

// this class will process each data stream from the sensor, and depending on the applied settings, process it further, to finally send it on the selected output interface

class Engine
{
    public:
        Engine();

        unordered_map<string, MidiTranslator> Miditranslators ={
            {"roll",MidiTranslator()},
            {"pitch",MidiTranslator()},
            {"yaw",MidiTranslator()},
            {"accX",MidiTranslator()},
            {"accY",MidiTranslator()},
            {"accZ",MidiTranslator()}
        };

        unordered_map<string, HidTranslator> hid_map ={
            {"roll",HidTranslator()},
            {"pitch",HidTranslator()},
            {"yaw",HidTranslator()},
            {"accX",HidTranslator()},
            {"accY",HidTranslator()},
            {"accZ",HidTranslator()}
        };

        // Have to move this into midi translator
        unordered_map<string, int> cc_map= {
            {"roll",1},
            {"pitch",2},
            {"yaw",3},
            {"accX",4},
            {"accY",5},
            {"accZ",6}
        };

        hid_gamepad_report_t    gp;
        hid_keyboard_report_t    kb;
        hid_mouse_report_t       mouse;


        void update(midi_io& midiio,usb_hid& hidio);
        void midi_processsor(midi_io& midiio);
        void hid_processor(usb_hid& hidio);
        void set_default_config();
        json get_config();
        void set_config(Config& config);

};


#endif //ENGINE_H