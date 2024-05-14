#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "input_sensor.h"

// find way to add mode
// might be done together with the change of structure to associate mode or output to inputs. 

class AnalogSensor : public Sensor{   
    public:

        AnalogSensor(){
            sensor_dat = {
            {"A1", {true, false, 0, 0, 0}},
            {"A2", {true, false, 0, 0, 0}},
            {"A3", {true, false, 0, 0, 0}},
            {"A4", {true, false, 0, 0, 0}},
            {"A5", {true, false, 0, 0, 0}},
            {"A6", {true, false, 0, 0, 0}},
            {"T1", {true, false, 0, 0, 0}},
            {"T2", {true, false, 0, 0, 0}},
            {"T3", {true, false, 0, 0, 0}},
            {"T4", {true, false, 0, 0, 0}},
            {"T5", {true, false, 0, 0, 0}},
            {"T6", {true, false, 0, 0, 0}}
        };
        };

        void init() override;
        void setup() override;
        void update() override;

    private:
        
};

#endif //ANALOG_SENSOR_H