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
                    {"roll",MidiTranslator(sensor.get_sensor_dat_map()["roll"].limit_max)},
                    {"pitch",MidiTranslator(sensor.get_sensor_dat_map()["pitch"].limit_max)},
                    {"yaw",MidiTranslator(sensor.get_sensor_dat_map()["yaw"].limit_max)},
                    {"accX",MidiTranslator(sensor.get_sensor_dat_map()["accX"].limit_max)},
                    {"accY",MidiTranslator(sensor.get_sensor_dat_map()["accY"].limit_max)},
                    {"accZ",MidiTranslator(sensor.get_sensor_dat_map()["accZ"].limit_max)}
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
                    {"dist",MidiTranslator(sensor.get_sensor_dat_map()["dist"].limit_max)}
                };
                hid_map ={
                    {"dist",HidTranslator()}
                };
            #elif defined(PIPO_ANALOG)
                Miditranslators ={
                    {"A1",MidiTranslator(sensor.get_sensor_dat_map()["A1"].limit_max)},
                    {"A2",MidiTranslator(sensor.get_sensor_dat_map()["A2"].limit_max)},
                    {"A3",MidiTranslator(sensor.get_sensor_dat_map()["A3"].limit_max)},
                    {"A4",MidiTranslator(sensor.get_sensor_dat_map()["A4"].limit_max)},
                    {"A5",MidiTranslator(sensor.get_sensor_dat_map()["A5"].limit_max)},
                    {"A6",MidiTranslator(sensor.get_sensor_dat_map()["A6"].limit_max)},
                    {"T1",MidiTranslator(sensor.get_sensor_dat_map()["T1"].limit_max)},
                    {"T2",MidiTranslator(sensor.get_sensor_dat_map()["T2"].limit_max)},
                    {"T3",MidiTranslator(sensor.get_sensor_dat_map()["T3"].limit_max)},
                    {"T4",MidiTranslator(sensor.get_sensor_dat_map()["T4"].limit_max)},
                    {"T5",MidiTranslator(sensor.get_sensor_dat_map()["T5"].limit_max)},
                    {"T6",MidiTranslator(sensor.get_sensor_dat_map()["T6"].limit_max)}
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