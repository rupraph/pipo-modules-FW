#include "engine.h"
#include "midi_translator.h"

sensor& mySensor = sensor::getInstance(); // Get the singleton instance


void engine_setup()
{

}

void engine_update(sensor mySensor)
{

    
}


void midi_processsor(sensor mySensor)
{
    for (auto const& x : mySensor.data_map)
    {
        if (x.second)
        {
            
        }
    }
}

