#include "sensors/sensors.h"

#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"
MotionSensor input_sensor;
#endif

#ifdef PIPO_RANGE
#include "sensors/pipo_range_sensor.h"
PipoRangeSensor input_sensor;
#endif

#ifdef PIPO_ANALOG
#include "sensors/analog_sensor.h"
AnalogSensor input_sensor;
#endif

#ifdef PIPO_MGC3130
#include "sensors/mgc3130_sensor.h"
MGC3130Sensor input_sensor;
#endif