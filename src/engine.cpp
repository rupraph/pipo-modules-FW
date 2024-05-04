#include "engine.h"



// for convenience
using json = nlohmann::json;

sensor& mySensor = sensor::getInstance(); // Get the singleton instance


// should create a table for axis config. min max, etc, since this is shared for both hid and midi


unordered_map<string, MidiTranslator> Miditranslators ={
    {"roll",MidiTranslator()},
    {"pitch",MidiTranslator()},
    {"yaw",MidiTranslator()},
    {"accX",MidiTranslator()},
    {"accY",MidiTranslator()},
    {"accZ",MidiTranslator()}
};

// Have to move this to midi translator
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

// Define a map from strings and functions that take a hid_gamepad_report_t& and return a reference to an int
// std::unordered_map<std::string, std::function<int8_t&(hid_gamepad_report_t&)>> gp_axis_map;
// std::unordered_map<std::string, std::function<int32_t&(hid_gamepad_report_t&)>> gp_button_map;

unordered_map<string, HidTranslator> hid_map ={
    {"roll",HidTranslator()},
    {"pitch",HidTranslator()},
    {"yaw",HidTranslator()},
    {"accX",HidTranslator()},
    {"accY",HidTranslator()},
    {"accZ",HidTranslator()}
};

// will have to add error catching: -> when config could not load for eg. 



void engine_setup()
{
    // load config
    
    
    //set_default_config();
    
    //test config
    json test = engine_get_config();
    //Serial.println(get_config().c_str());

}

void set_default_config()
{
    // Hid mapping config
    // gp_axis_map["roll"] = [](hid_gamepad_report_t& gp) -> int8_t& { return gp.x; };
    // gp_axis_map["pitch"] = [](hid_gamepad_report_t& gp) -> int8_t& { return gp.y; };
    hid_map["roll"].mapto = "x";
    hid_map["pitch"].mapto = "y";


    // Midi mapping config
    Miditranslators["roll"].max_input=180;
    Miditranslators["roll"].min_input=-180;
    Miditranslators["roll"].translator_mode=1;
    Miditranslators["roll"].rootNote=40;
    //Miditranslators["roll"].printScale(Miditranslators["roll"].current_scale);
    Miditranslators["roll"].update_scale();


    Miditranslators["pitch"].max_input=90;
    Miditranslators["pitch"].min_input=-90;

    Miditranslators["yaw"].max_input=180;
    Miditranslators["yaw"].min_input=-180;

    // ADD SAVE CONFIG
}

json engine_get_config()
{   
        json jengine;
        json j;
        for (auto const& pair : Miditranslators)
        {
            jengine["engine-midi"][pair.first] = pair.second.get_json();
        }
        for (auto const& pair : hid_map)
        {
            jengine["engine-hid"][pair.first] = pair.second.get_json();
        }
        j["engine"] = jengine;
        Serial.println(j.dump().c_str());
        return j;
}

void engine_set_config(Config& config)
{
    json jengine = config.get_config_for_key("engine");
    json jmidi = jengine["engine-midi"];
    // set midi config from general config
    for (auto const& pair : Miditranslators)
    {
        if (jmidi.find(pair.first) != jmidi.end())
        {
            Miditranslators[pair.first].set_from_json(jmidi[pair.first]);
        }
    }
    // set hid config from general config
    json jhid = jengine["engine-hid"];
    for (auto const& pair : hid_map)
    {
        if (jhid.find(pair.first) != jhid.end())
        {
            hid_map[pair.first].set_from_json(jhid[pair.first]);
        }
    }
}

void load_config(String filename)
{
    // string config = readFile(LittleFS,filename.c_str());
    // //Serial.println(config.c_str());
    // set_config(config);
}


void engine_update(midi_io& midiio,usb_hid& hidio)
{
    midi_processsor(midiio);
    hid_processor(hidio);
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

void hid_processor(usb_hid& hidio)
{ // not dealing with buttons yet

    for (auto const& pair : mySensor.enable_map)
    {
        if (pair.second)
        {
            string name=pair.first;
            float sensor_val=mySensor.data_map[name];

            if (hid_map.find(name) != hid_map.end())
            {
                string map_name=hid_map[name].mapto;
                int hid_val=hid_map[name].get_current_int(sensor_val);
                if (hidio.hid_mode==0)
                {
                    if (map_name=="x")
                    {
                        gp.x=hid_val;
                    }
                    else if (map_name=="y")
                    {
                        gp.y=hid_val;
                    }
                    else if (map_name=="z")
                    {
                        gp.z=hid_val;
                    }
                    else if (map_name=="rx")
                    {
                        gp.rx=hid_val;
                    }
                    else if (map_name=="ry")
                    {
                        gp.ry=hid_val;
                    }
                    else if (map_name=="rz")
                    {
                        gp.rz=hid_val;
                    }
                }
                else if (hidio.hid_mode==1)
                {
                    if (map_name=="x")
                    {
                        mouse.x=hid_val;
                    }
                    else if (map_name=="y")
                    {
                        mouse.y=hid_val;
                    }
                    else if (map_name=="wheel")
                    {
                        mouse.wheel=hid_val;
                    }
                }
            }
        }
    }

    // should move report in engine.
    switch (hidio.hid_mode)
    {
    case 0:
        hidio.usb_hid_update(&gp);
        break;
    case 1:
        hidio.usb_hid_update(&mouse);
        break;
    case 2:
        hidio.usb_hid_update(&kb);
        break;
    }
}



