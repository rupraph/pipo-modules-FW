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

#ifdef PIPO_MGC3130
#include "sensors/mgc3130_sensor.h"
extern MGC3130Sensor input_sensor;
#endif

#endif  // PIPO_SENSORS