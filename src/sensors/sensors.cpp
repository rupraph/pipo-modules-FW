#include "sensors/sensors.h"

#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"
MotionSensor input_sensor;
#endif

#ifdef PIPO_RANGE
#include "sensors/pipo_range_sensor.h"
PipoRangeSensor input_sensor;
#endif

#ifdef PIPO_RANGE_L5CX
#include "sensors/pipo_range_sensor_l5cx.h"
PipoRangeSensorL5CX input_sensor;
#endif

#ifdef PIPO_ANALOG
#include "sensors/analog_sensor.h"
AnalogSensor input_sensor;
#endif