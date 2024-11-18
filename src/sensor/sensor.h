#ifndef PIPO_MOTION_GRGRGRG
#define PIPO_MOTION_GRGRGRG

#ifdef PIPO_MOTION
#include "sensor/acc_sensor.h"
// string sensor_type = "motion";
extern MotionSensor input_sensor;
#endif
#ifdef PIPO_RANGE
#include "sensor/range_sensor.h"
extern RangeSensor input_sensor;
// string sensor_type = "range";
#endif
#ifdef PIPO_ANALOG
#include "sensor/analog_sensor.h"
extern AnalogSensor input_sensor;
// string sensor_type = "analog";
#endif

#endif  // PIPO_MOTION_GRGRGRG