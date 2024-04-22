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

void engine_setup()
{
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
                Serial.print("sensor_val:");
                Serial.println(note_val);
                // should probably move the value check in the io class. to be discussed
                midiio.sendNoteOn(note_val,127,1,800); 
            }
        }
    }
    
}

