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

    measure_offset_all();

}


//Todo: add function to perform individual offset or of provided list

void AnalogSensor::measure_offset_all()
{
    // perform intial baseline calibration
    int num_samples = 50;
    unordered_map<string,float> offset;
    for (int i = 0; i < num_samples; i++)
    {
        
        // measure values without corretcing for offset
        for (auto const& pair : analog_map)
        {
            sensor_dat[pair.first].value = analogRead(pair.second);
        }
        for (auto const& pair : touch_map)
        {
            sensor_dat[pair.first].value = touchRead(pair.second);
        }


        // save measurement
        for (auto const& pair : sensor_dat)
        {
            offset[pair.first] += sensor_dat[pair.first].value;
        }
        delay(20);
    }
    for (auto const& pair : sensor_dat)
    {
        sensor_dat[pair.first].offset = offset[pair.first] / num_samples;
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
        float val = lp_filter_map[pair.first].process(touchRead(pair.second))-sensor_dat[pair.first].offset;

        // if (value>sensor_dat[pair.first].limit_max && touch_adaptative_max)
        // {
        //     sensor_dat[pair.first].value = value;
        //     sensor_dat[pair.first].limit_max = value;
            // Todo: should find a way that changing the sensor limit also propagates to the connected midi translator
        // }
        // else
        // if (val>sensor_dat[pair.first].limit_max)// && !touch_adaptative_max)
        // {
        //     sensor_dat[pair.first].value = sensor_dat[pair.first].limit_max;
        // }
        // else if (val<sensor_dat[pair.first].limit_min)
        // {
        //     sensor_dat[pair.first].value = sensor_dat[pair.first].limit_min;
        // }
        // else
        // {
        sensor_dat[pair.first].value_prev = sensor_dat[pair.first].value;
        sensor_dat[pair.first].value = val;
        
        
        if (val>sensor_dat[pair.first].limit_min  
        && sensor_dat[pair.first].value_prev<sensor_dat[pair.first].limit_min )
        {
            if (!sensor_dat[pair.first].triggered)
            {
                sensor_dat[pair.first].triggered = true;
            }
        }
        else if (val<sensor_dat[pair.first].limit_min
        && sensor_dat[pair.first].value_prev>sensor_dat[pair.first].limit_min)
        {
            if (!sensor_dat[pair.first].untriggered)
            {
                sensor_dat[pair.first].untriggered = true;
            }
        }
        else
        {
            sensor_dat[pair.first].triggered = false;
            sensor_dat[pair.first].untriggered = false;
        }
        // }

    }
}


// generic functions /////////////////


