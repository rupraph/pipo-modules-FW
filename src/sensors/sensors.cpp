#include "sensors/sensors.h"

#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"
MotionSensor input_sensor;
#endif

#ifdef PIPO_RANGE
#include "sensors/range_sensor.h"
RangeSensor input_sensor;
#endif

#ifdef PIPO_ANALOG
#include "sensors/analog_sensor.h"
#include "sensors/analog_out.h"
AnalogSensor input_sensor;
AnalogOut analog_out;
#endif