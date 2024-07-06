#include "engine.h"
#include "HW_CONFIG.h"

// for convenience
using json = nlohmann::json;

// Todo engine. 
// should create a table for axis config. min max, etc, since this is shared for both hid and midi
// should push axis enabling in here instead of in the sensor class
// will have to add error catching: -> when config could not load for eg. 
// could use combination mode to have note from orientation, and trigger from acceleration
// find way to map scales/arpegio over axis (for analog when touch is a note)
// should use callback to trigger events and notes
// find way to:
            //- limit max sending freq
            //- send on change only -> in midi io ? 
            //- play / pause

void Engine::update(Sensor& sensor, midi_io& midiio,usb_hid& hidio)
{
    midiio.manage_sustain();
    midi_processsor(sensor, midiio);
    hid_processor(sensor, hidio);
}


void Engine::midi_processsor(Sensor& sensor, midi_io& midiio)
{
    // loop through sensor data
    const auto& sensor_dat = sensor.get_sensor_dat_map();
    for (auto const& pair : sensor_dat)
    {
        string axis_name=pair.first;
        float sensor_val=sensor.get_value(axis_name);
        int channel=Miditranslators[axis_name].channel;

        //check if axis is enabled, outside deadzone and not disabled
        if (sensor.get_enabled(axis_name) 
        && sensor.test_outside_deadzone(axis_name) 
        && Miditranslators[axis_name].disabled==false) //Todo: temporary. should likely have "in_use" to trigger note on/off
        {
        // if CC MODE
            if (Miditranslators[axis_name].translator_mode==0)
            {   
                //Todo: hires not tested
                int cc_number=Miditranslators[axis_name].cc_number;
                if (Miditranslators[axis_name].getHires())
                {
                    uint16_t cc_val=max(0,min(Miditranslators[axis_name].get_cc_val(sensor_val,1),16383));
                    midiio.sendControlChange(cc_number, cc_val, channel,true);

                }
                else
                {
                    uint8_t cc_val=max(0,min(Miditranslators[axis_name].get_cc_val(sensor_val,0),127));
                    // for midi find way to limit rotation to max 180° to avoid overflow to 0
                    midiio.sendControlChange(cc_number, cc_val, channel,false);

                }
         
            }

            

        // if Note mode
            else
            {   
                note_val_prev[channel]=note_val[channel];
                note_val[channel]=max(0,min(Miditranslators[axis_name].get_note(sensor_val),127));
                
                // probaly get triggered should be something linked to the deadzone
                #if defined(PIPO_ANALOG)
                // was this written only for sending single notes ? 
                if (sensor.get_triggered(axis_name))
                {
                    midiio.sendNoteOn(note_val,127,channel,20000); 
                    // reset trigger when note is sent
                    sensor.set_triggered(axis_name,false);
                }
                else if (sensor.get_untriggered(axis_name))
                {
                    midiio.sendNoteOff(note_val,127,channel);
                    sensor.set_untriggered(axis_name,false);
                }
                #endif

                # if defined(PIPO_MOTION)
                if (note_val[channel]!=note_val_prev[channel])
                {
                    midiio.sendNoteOn(note_val[channel],127,channel,20000); 
                }
                #endif


                #if defined(PIPO_RANGE)
                //Part of this logic migth have to move to sensor ?
                if (sensor.get_triggered(axis_name) && sensor_val<sensor.get_limit_max(axis_name))
                {
                    // should probably move the value check in the io class. to be discussed
                    midiio.sendNoteOn(note_val[channel],127,channel,800); 
                    sensor.set_triggered(axis_name,false);
                }
                if (sensor_val<sensor.get_limit_max(axis_name) && midiio.channel_note_list[channel].find(note_val[channel]) == midiio.channel_note_list[channel].end())
                {
                    midiio.sendNoteOn(note_val[channel],127,channel,800);    
                }
                //deal with NoteOff for range (== to max) 
                if (sensor_val==sensor.get_limit_max(axis_name))
                {
                    midiio.sendAllNotesOff(channel);
                }

                #endif
                
            }
        }
    }
    
}

void Engine::hid_processor(Sensor& sensor,usb_hid& hidio)
{ // not dealing with buttons yet
    
    const auto& sensor_dat = sensor.get_sensor_dat_map();
    
    for (auto const& pair : sensor_dat)
    {
        string axis_name=pair.first;
        float sensor_val=sensor.get_value(axis_name);

        if (sensor.get_enabled(axis_name) && hid_map[axis_name].disabled==false)
        {
            if (hid_map.find(axis_name) != hid_map.end())
            {
                string map_name=hid_map[axis_name].mapto;
                int hid_val=hid_map[axis_name].get_current_int(sensor_val);
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
    // // Hid mapping config
    // hid_map["roll"].mapto = "x";
    // hid_map["pitch"].mapto = "y";


    // // Midi mapping config
    // Miditranslators["roll"].max_input=180;
    // Miditranslators["roll"].min_input=-180;
    // Miditranslators["roll"].translator_mode=1;
    // Miditranslators["roll"].rootNote=40;
    // //Miditranslators["roll"].printScale(Miditranslators["roll"].current_scale);
    // Miditranslators["roll"].update_scale();


    // Miditranslators["pitch"].max_input=90;
    // Miditranslators["pitch"].min_input=-90;

    // Miditranslators["yaw"].max_input=180;
    // Miditranslators["yaw"].min_input=-180;

    // // ADD SAVE CONFIG
}

json Engine::get_config(bool debug)
{   
        json j;
        for (auto const& pair : Miditranslators)
        {
            j["engine-midi"][pair.first] = pair.second.get_json();
        }
        for (auto const& pair : hid_map)
        {
            j["engine-hid"][pair.first] = pair.second.get_json();
        }
        if (debug)
        {
            Serial.println("engine_get_config");
            Serial.println(j.dump(4).c_str());
            Serial.println("engine_get_config_end");
        } 
            
        return j;
}





void Engine::set_config(json& config, bool debug)
{   
    if (debug){
        Serial.println("will set config");
        Serial.println(config.dump().c_str());
        Serial.println();
    }
    
    json jmidi = config["engine-midi"];
    
    // set midi config from general config
    for (auto const& pair : Miditranslators)
    {
        if (jmidi.find(pair.first) != jmidi.end())
        {
            // Serial.println(jmidi[pair.first].dump().c_str());
            // Serial.println(pair.first.c_str());
            Miditranslators[pair.first].set_from_json(jmidi[pair.first]);
        }
    }
    Serial.println("midi config set");
    // set hid config from general config
    json jhid = config["engine-hid"];
    for (auto const& pair : hid_map)
    {
        if (jhid.find(pair.first) != jhid.end())
        {
            hid_map[pair.first].set_from_json(jhid[pair.first]);
        }
    }
    Serial.println("hid config set");
}
