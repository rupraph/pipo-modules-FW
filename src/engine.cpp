#include "engine.h"


// for convenience
using json = nlohmann::json;

sensor& mySensor = sensor::getInstance(); // Get the singleton instance


// should create a table for axis config. min max, etc, since this is shared for both hid and midi

// should push axis enabling in here instead of in the sensor class

// will have to add error catching: -> when config could not load for eg. 

Engine::Engine()
{

}

void Engine::update(midi_io& midiio,usb_hid& hidio)
{
    midi_processsor(midiio);
    hid_processor(hidio);
}


void Engine::midi_processsor(midi_io& midiio)
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

void Engine::hid_processor(usb_hid& hidio)
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

void Engine::set_default_config()
{
    // Hid mapping config
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

json Engine::get_config()
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

void Engine::set_config(Config& config)
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

