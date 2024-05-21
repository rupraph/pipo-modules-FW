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

    // perform intial baseline calibration
    unordered_map<string,float> offset;
    for (int i = 0; i < 10; i++)
    {
        update();
        for (auto const& pair : sensor_dat)
        {
            offset[pair.first] += sensor_dat[pair.first].value;
        }
        delay(10);
    }
    for (auto const& pair : sensor_dat)
    {
        sensor_dat[pair.first].offset = offset[pair.first] / 10;
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
        float value = lp_filter_map[pair.first].process(touchRead(pair.second)-sensor_dat[pair.first].offset);
        
        // if (value>sensor_dat[pair.first].limit_max && touch_adaptative_max)
        // {
        //     sensor_dat[pair.first].value = value;
        //     sensor_dat[pair.first].limit_max = value;
            // Todo: should find a way that changing the sensor limit also propagates to the connected midi translator
        // }
        // else
        if (value>sensor_dat[pair.first].limit_max && !touch_adaptative_max)
        {
            sensor_dat[pair.first].value = sensor_dat[pair.first].limit_max;
        }
        else
        {
            sensor_dat[pair.first].value = value;
        }

    }
}


// generic functions /////////////////


