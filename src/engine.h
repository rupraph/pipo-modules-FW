#ifndef ENGINE_H
#define ENGINE_H

#include "acc_sensor.h"

// this class will process each data stream from the sensor, and depending on the applied settings, process it further, to finally send it on the selected output interface


void engine_setup();
void engine_update();
void midi_processsor();



#endif //ENGINE_H