#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "sensor/input_sensor.h"
#include "utils/filters.h"
#include "HW_CONFIG.h"

// find way to add/change mode

using namespace std;

class AnalogSensor : public Sensor{   
    public:
        AnalogSensor(){
            sensor_dat = {
            {"A1", {true, false, 0, 0, 0, 4095, false}},
            {"A2", {true, false, 0, 0, 0, 4095, false}},
            {"A3", {true, false, 0, 0, 0, 4095, false}},
            {"A4", {true, false, 0, 0, 0, 4095, false}},
            {"A5", {true, false, 0, 0, 0, 4095, false}},
            {"A6", {true, false, 0, 0, 0, 4095, false}},
            {"T1", {true, false, 0, 0, 0, 50000, false}},
            {"T2", {true, false, 0, 0, 0, 50000, false}},
            {"T3", {true, false, 0, 0, 0, 50000, false}},
            {"T4", {true, false, 0, 0, 0, 50000, false}},
            {"T5", {true, false, 0, 0, 0, 50000, false}},
            {"T6", {true, false, 0, 0, 0, 50000, false}}
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
        void measure_offset_all();

    private:
            unordered_map<string, LowPassFilter> lp_filter_map = {
            {"A1", LowPassFilter(10)},
            {"A2", LowPassFilter(10)},
            {"A3", LowPassFilter(10)},
            {"A4", LowPassFilter(10)},
            {"A5", LowPassFilter(10)},
            {"A6", LowPassFilter(10)},
            {"T1", LowPassFilter(10)},
            {"T2", LowPassFilter(20)},
            {"T3", LowPassFilter(30)},
            {"T4", LowPassFilter(5)},
            {"T5", LowPassFilter(5)},
            {"T6", LowPassFilter(5)}
        };

        

        bool touch_adaptative_max;

        
};

#endif //ANALOG_SENSOR_H