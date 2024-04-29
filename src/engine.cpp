#include "engine.h"

sensor& mySensor = sensor::getInstance(); // Get the singleton instance

unordered_map<string, MidiTranslator> Miditranslators ={
    {"roll",MidiTranslator()},
    {"pitch",MidiTranslator()},
    {"yaw",MidiTranslator()},
    {"accX",MidiTranslator()},
    {"accY",MidiTranslator()},
    {"accZ",MidiTranslator()}
};

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

// Define a map from strings to functions that take a hid_gamepad_report_t& and return a reference to an int
std::unordered_map<std::string, std::function<int8_t&(hid_gamepad_report_t&)>> gp_axis_map;
std::unordered_map<std::string, std::function<int32_t&(hid_gamepad_report_t&)>> gp_button_map;


void engine_setup()
{
    usb_hid_setup();

    // Hid mapping config
    gp_axis_map["roll"] = [](hid_gamepad_report_t& gp) -> int8_t& { return gp.x; };
    gp_axis_map["pitch"] = [](hid_gamepad_report_t& gp) -> int8_t& { return gp.y; };

    // Midi mapping config
    Miditranslators["roll"].max_input=180;
    Miditranslators["roll"].min_input=-180;
    Miditranslators["roll"].translator_mode=1;
    Miditranslators["roll"].rootNote=40;
    Miditranslators["roll"].printScale(Miditranslators["roll"].current_scale);
    Miditranslators["roll"].update_scale();


    Miditranslators["pitch"].max_input=90;
    Miditranslators["pitch"].min_input=-90;

    Miditranslators["yaw"].max_input=180;
    Miditranslators["yaw"].min_input=-180;

}

void engine_update(midi_io& midiio)
{
    midi_processsor(midiio);
    hid_processor();
    
}


void midi_processsor(midi_io& midiio)
{
    for (auto const& pair : mySensor.enable_map)
    {
        if (pair.second)
        {
            string name=pair.first;
            float sensor_val=mySensor.data_map[name];

            if (Miditranslators[name].translator_mode==0)
            {   
                uint8_t cc_val=max(0,min(Miditranslators[name].get_cc_val(sensor_val,0),127));
                // for midi find way to limit rotation to max 180° to avoid overflow to 0
                midiio.sendControlChange(cc_map[name], cc_val, 1,false);

                // if debug ? 
                // Serial.print("cc ");
                // Serial.print(name.c_str());
                // Serial.print(": ");
                // Serial.println(mySensor.data_map[name]);
                //Serial.print(" :");
                //Serial.print(Miditranslators[name].get_cc_val(mySensor.data_map[name]));
         
            }

            // find way to:
            //- limit max sending freq
            //- send on change only -> in midi io ? 
            //- play / pause

            else
            {   
                
                uint8_t note_val=max(0,min(Miditranslators[name].get_note(sensor_val),127));
                // Serial.print("sensor_val:");
                // Serial.println(note_val);
                // should probably move the value check in the io class. to be discussed
                midiio.sendNoteOn(note_val,127,1,800); 
            }
        }
    }
    
}

void hid_processor()
{
    for (auto const& pair : mySensor.enable_map)
    {
        if (pair.second)
        {
            string name=pair.first;
            float sensor_val=mySensor.data_map[name];

            if (gp_axis_map.find(name) != gp_axis_map.end())
            {
                // Serial.print("hid ");
                // Serial.println(map(sensor_val, -180, 180, -127, 127));
                gp_axis_map[name](gp)= map(sensor_val, -180, 180, -127, 127);
            }
        }
    }

    //gp.x = 125;
    usb_hid_update(&gp);

}

