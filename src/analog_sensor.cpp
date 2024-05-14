#include "analog_sensor.h"


void AnalogSensor::init()
{

}

void AnalogSensor::setup()
{
    for (auto const& pair : analog_map)
    {
        pinMode(pair.second, INPUT);
    }
    for (auto const& pair : touch_map)
    {
        pinMode(pair.second, INPUT);
    }
}

void AnalogSensor::update()
{
    for (auto const& pair : analog_map)
    {
        sensor_dat[pair.first].value = analogRead(pair.second);
    }
    for (auto const& pair : touch_map)
    {
        sensor_dat[pair.first].value = touchRead(pair.second);
    }
}


// generic functions /////////////////


