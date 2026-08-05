#include "sensors/sensors.h"

#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"
MotionSensor input_sensor;
#endif

#ifdef PIPO_RANGE
#include "sensors/pipo_range_sensor.h"
PipoRangeSensor input_sensor;
#endif

#ifdef PIPO_MAX30102
#include "sensors/max30102_sensor.h"
Max30102Sensor input_sensor;
#endif

#ifdef PIPO_ANALOG
#include "sensors/analog_sensor.h"
AnalogSensor input_sensor;
#endif