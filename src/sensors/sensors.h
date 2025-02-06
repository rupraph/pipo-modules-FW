#ifndef PIPO_SENSORS
#define PIPO_SENSORS

#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"
// string sensor_type = "motion";
extern MotionSensor input_sensor;
#endif

#ifdef PIPO_RANGE
#include "sensors/range_sensor.h"
extern RangeSensor input_sensor;
// string sensor_type = "range";
#endif

#ifdef PIPO_ANALOG
#include "sensors/analog_sensor.h"
#include "sensors/analog_out.h"
extern AnalogSensor input_sensor;
extern AnalogOut analog_out;
// string sensor_type = "analog";
#endif

#endif  // PIPO_SENSORS