#include "engine.h"
#include "HW_CONFIG.h"


using json = nlohmann::json;

//the engine takes the sensor data and outputs it to the selected interfaces based on the configuration

// Todo engine. 
// could use combination mode to have note from orientation, and trigger from acceleration

void Engine::update(Sensor& sensor, midi_io& midiio,usb_hid& hidio,OSC_handler& osc)
{
    if (config.general_config["MidiEnabled"]==true)
    {
        midiio.manage_sustain();
        midi_processor(sensor, midiio);
    }
    if (config.general_config["OSC_ENA"]==true)
    {
        osc_processor(sensor,osc);
    }
    if (config.general_config["HidEnabled"]==true)
    {
        hid_processor(sensor, hidio);
    }
}



void Engine::midi_processor(Sensor& sensor, midi_io& midiio)
{
    // loop through sensor data
    const auto& sensor_dat = sensor.get_sensor_dat_map();
    for (auto const& pair : sensor_dat)
    {
        string axis_name=pair.first;
        float sensor_val=sensor.get_value(axis_name);
        float sensor_min=sensor.get_limit_min(axis_name);
        float sensor_max=sensor.get_limit_max(axis_name);
        int channel=Miditranslators[axis_name].channel;

        //check if axis is enabled, outside deadzone and not disabled
        if (sensor.get_enabled(axis_name) 
        && sensor.test_outside_deadzone(axis_name) 
        && Miditranslators[axis_name].enabled==true) //Todo: temporary. should likely have "in_use" to trigger note on/off
        {
        // if CC MODE
            if (Miditranslators[axis_name].translator_mode==0)
            {   
                //Todo: hires not tested
                int cc_number=Miditranslators[axis_name].cc_number;
                if (Miditranslators[axis_name].getHires())
                {
                    uint16_t cc_val=max(0,min(Miditranslators[axis_name].get_cc_val(sensor_val,sensor_min,sensor_max,1),16383));
                    midiio.sendControlChange(cc_number, cc_val, channel,true);
                    
                }
                else
                {
                    uint8_t cc_val=max(0,min(Miditranslators[axis_name].get_cc_val(sensor_val,sensor_min,sensor_max,0),127));
                    midiio.sendControlChange(cc_number, cc_val, channel,false);

                }
                vTaskDelay(pdTICKS_TO_MS(5)); // virtually delay cc send. will be solved with task management 
         
            }

        // if Note mode
            else
            {   
                note_val_prev[channel]=note_val[channel];
                int note=(Miditranslators[axis_name].get_note(sensor_val,sensor_min,sensor_max));
                note_val[channel]=max(0,min(note,127)); //clip between 0 and 127
                
                // probaly get triggered should be something linked to the deadzone
                #if defined(PIPO_ANALOG)
                // trigger new note if within range, not already playing, and new note is different from previous note
                if (sensor.is_within_range(axis_name)
                && note_val[channel]!=note_val_prev[channel]
                && !midiio.is_note_playing(note_val[channel],channel))
                {
                    midiio.sendNoteOn(note_val[channel],127,channel,3000); 
                }

                //if entering range, send new note
                if (sensor.get_triggered(axis_name))
                {
                    midiio.sendNoteOn(note_val[channel],127,channel);
                    sensor.set_triggered(axis_name,false);
                }

                //if exiting range, send note off
                if (sensor.get_untriggered(axis_name))
                {
                    midiio.sendAllNotesOff(channel);
                    sensor.set_untriggered(axis_name,false);
                }
                #endif

                # if defined(PIPO_MOTION)
                if (note_val[channel]!=note_val_prev[channel])
                {
                    midiio.sendNoteOn(note_val[channel],127,channel,5000); 
                }
                #endif


                #if defined(PIPO_RANGE) // to be rewritten. not clear split between what shall be in sensor and what in engine
                //Part of this logic migth have to move to sensor ?
                if (sensor.get_triggered(axis_name) && sensor_val<sensor.get_limit_max(axis_name))
                {
                    // should probably move the value check in the io class. to be discussed
                    midiio.sendNoteOn(note_val[channel],127,channel,800); 
                    sensor.set_triggered(axis_name,false);
                }
                if (sensor_val<sensor.get_limit_max(axis_name) && !midiio.is_note_playing(note_val[channel],channel))//midiio.channel_note_list[channel].find(note_val[channel]) == midiio.channel_note_list[channel].end())
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
        float sensor_min=sensor.get_limit_min(axis_name);
        float sensor_max=sensor.get_limit_max(axis_name);

        if (sensor.get_enabled(axis_name) && hid_map[axis_name].enabled==true)
        {
            if (hid_map.find(axis_name) != hid_map.end())
            {
                string map_name=hid_map[axis_name].mapto;
                int hid_val=hid_map[axis_name].get_current_int(sensor_val,sensor_min,sensor_max);
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

void Engine::osc_processor(Sensor& sensor,OSC_handler& osc)
{
    // Todo: loop through sensor data -> indentical for 3 processor, should be factorized

        const auto& sensor_dat = sensor.get_sensor_dat_map();
        for (auto const& pair : sensor_dat)
        {
            string axis_name=pair.first;
            float sensor_val=sensor.get_value(axis_name);
            float sensor_min=sensor.get_limit_min(axis_name);
            float sensor_max=sensor.get_limit_max(axis_name);
            if (sensor.get_enabled(axis_name) 
            && Osctranslators[axis_name].enabled
            && sensor.test_outside_deadzone(axis_name))
            {
                //Serial.print(axis_name.c_str());
                //Serial.println(sensor_val);
                float osc_val=Osctranslators[axis_name].get_value(sensor_val,sensor_min,sensor_max);
                //Serial.println(osc_val);
                osc.sendOscMessage(axis_name,osc_val);
            }
        }
        
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
        for (auto const& pair : Osctranslators)
        {
            j["engine-osc"][pair.first] = pair.second.get_json();
        }
        if (debug)
        {
            Serial.println(F("engine_get_config"));
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
    
    // set midi config from main config
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

    json josc = config["engine-osc"];
    for (auto const& pair : Osctranslators)
    {
        if (josc.find(pair.first) != josc.end())
        {
            Osctranslators[pair.first].set_from_json(josc[pair.first]);
        }
    }
    Serial.println("osc config set");
}
