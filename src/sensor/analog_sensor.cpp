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
    #ifdef DEBUG_HEAP
        Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
    #endif
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
            sensor_dat[pair.first].value = analogRead(pair.second)*0.000806;
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
        sensor_dat[pair.first].offset = round((offset[pair.first] / num_samples) * 1000.0) / 1000.0;
    }
}


void AnalogSensor::update()
{
    for (auto const& pair : analog_map)
    {
       float analog_val = analogRead(pair.second)*0.000806; //convert to volts

        sensor_dat[pair.first].value_prev = sensor_dat[pair.first].value;
        sensor_dat[pair.first].value =  analog_val;
        
        
        if (analog_val>sensor_dat[pair.first].limit_min  
        && sensor_dat[pair.first].value_prev<sensor_dat[pair.first].limit_min )
        {
            if (!sensor_dat[pair.first].triggered)
            {
                sensor_dat[pair.first].triggered = true;
            }
        }
        else if (analog_val<sensor_dat[pair.first].limit_min
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

    }
    for (auto const& pair : touch_map)
    {
        //trial filtering values
        //float val = lp_filter_map[pair.first].process(touchRead(pair.second))-sensor_dat[pair.first].offset;
        float val = touchRead(pair.second)-sensor_dat[pair.first].offset;

        //Todo: try to have an adaptative max ? 

        // should be generic for both or any sensor type ? 
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



