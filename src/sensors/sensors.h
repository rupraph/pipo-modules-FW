#ifndef PIPO_SENSORS
#define PIPO_SENSORS

#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"
extern MotionSensor input_sensor;
#endif

#ifdef PIPO_RANGE
#include "sensors/pipo_range_sensor.h"
extern PipoRangeSensor input_sensor;
#endif

#ifdef PIPO_ANALOG
#include "sensors/analog_sensor.h"
extern AnalogSensor input_sensor;
#endif

#endif  // PIPO_SENSORS