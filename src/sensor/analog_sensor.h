#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "sensor/input_sensor.h"
#include "HW_CONFIG.h"

// find way to add/change mode

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

        unordered_map<string, int> analog_map = {
            {"A1", 16},
            {"A2", 15},
            {"A3", 14},
            {"A4", 13},
            {"A5", 12},
            {"A6", 11}
        };

        unordered_map<string, int> touch_map = {
            {"T1", 6},
            {"T2", 5},
            {"T3", 4},
            {"T4", 3},
            {"T5", 2},
            {"T6", 1}
        };

        void init() override;
        void setup() override;
        void update() override;

        
};

#endif //ANALOG_SENSOR_H