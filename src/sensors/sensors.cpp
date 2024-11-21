#include "sensors/sensors.h"

#ifdef PIPO_MOTION
MotionSensor input_sensor;
#endif
#ifdef PIPO_RANGE
RangeSensor input_sensor;
#endif
#ifdef PIPO_ANALOG
AnalogSensor input_sensor;
#endif