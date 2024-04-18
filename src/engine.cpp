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

}

void engine_update(midi_io midiio)
{
    midi_processsor(midiio);
}


void midi_processsor(midi_io midiio)
{
    for (auto const& pair : mySensor.enable_map)
    {
        if (pair.second)
        {
            string name=pair.first;
            if (Miditranslators[name].translator_mode==0)
            {
                midiio.sendControlChange(cc_map[name], Miditranslators[name].get_cc_val(mySensor.data_map[name]), 1);

                // if debug ? 
                Serial.print("cc ");
                Serial.print(name.c_str());
                Serial.print(": ");
                Serial.println(mySensor.data_map[name]);
                //Serial.print(" :");
                //Serial.print(Miditranslators[name].get_cc_val(mySensor.data_map[name]));
                
                
                
            }

            // find way to:
            //- limit max sending freq
            //- send on change only -> in midi io ? 
            //- play / pause

            // else
            // {
            //     midiio.sendNoteOn(Miditranslators[name].get_note(mySensor.),127,1) 
            // }
        }
    }
    Serial.println();
}

