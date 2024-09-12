#ifndef RANGE_SENSOR_H
#define RANGE_SENSOR_H

#include "sensor/input_sensor.h"
#include "HW_CONFIG.h"
#include <vl53l4cx_class.h>
#include "utils/filters.h"



class RangeSensor : public Sensor{   
    public:
        RangeSensor(){
            sensor_dat["dist"] = SensorDat();

            // sensor_dat["dist"].deadzone = 0.0;
            // sensor_dat["dist"].offset = 0.0;
            // sensor_dat["dist"].value = 0.0;
            // sensor_dat["dist"].value_prev = 0.0;
            // sensor_dat["dist"].limit_max = 1000.0;
            // sensor_dat["dist"].limit_min = 0.0;
            // sensor_dat["dist"].triggered = false;
            // sensor_dat["dist"].untriggered = false;
            // sensor_dat["dist"].mode = 0;
            // sensor_dat["dist"].threshold_mode = 0;
            // sensor_dat["dist"].threshold = 0.0;
            // sensor_dat["dist"].bool_value = false;
        };

        void init() override;
        void setup() override;
        void update() override;

    private:

        bool within_range=false;
        bool within_range_prev=false;

        VL53L4CX vl53l4cx;
        VL53L4CX_MultiRangingData_t MultiRangingData;
        VL53L4CX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
        uint8_t NewDataReady;
        int no_of_object_found;
        char report[64];
        int status;

        SensorDat prev_sensor_dat;

        LowPassFilter lp_filter;
        MovingAverageFilter ma_filter;
        KalmanFilter km_filter;
};

#endif //RANGE_SENSOR_H