#include "engine.h"
#include "midi_io.h"


sensor& mySensor = sensor::getInstance(); // Get the singleton instance

unordered_map<string, MidiTranslator> Miditranslators ={
    {"roll",MidiTranslator()},
    {"pitch",MidiTranslator()},
    {"yaw",MidiTranslator()},
    {"accX",MidiTranslator()},
    {"accY",MidiTranslator()},
    {"accZ",MidiTranslator()}
};

void engine_setup()
{

}

void engine_update()
{

    
}


void midi_processsor(midi_io midiio)
{
    for (auto const& pair : mySensor.data_map)
    {
        if (pair.second)
        {
            string name=pair.first;
            if (Miditranslators[name].translator_mode==0)
            {
                midiio.sendControlChange(1, Miditranslators[name].get_cc_val(mySensor.data_map[name]), 1);
            }

            // find way to:
            //- limit max sending freq
            //- send on change
            //- play / pause

            // else
            // {
            //     midiio.sendNoteOn(Miditranslators[name].get_note(mySensor.),127,1) 
            // }
        }
    }
}

