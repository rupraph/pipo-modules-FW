#ifndef RANGE_SENSOR_H
#define RANGE_SENSOR_H

#include "input_sensor.h"
#include <vl53l4cx_class.h>


//todo missing capital at sensor
class RangeSensor : public Sensor{   
    public:
        // range_sensor(const range_sensor&) = delete;
        // void operator=(const range_sensor&) = delete;
        RangeSensor(){
            sensor_dat = {
            {"dist", {true, false, 0, 0, 0}}
        };
        };

        void init() override;
        void setup() override;
        void update() override;

    private:

        VL53L4CX vl53l4cx;
        VL53L4CX_MultiRangingData_t MultiRangingData;
        VL53L4CX_MultiRangingData_t *pMultiRangingData = &MultiRangingData;
        uint8_t NewDataReady;
        int no_of_object_found;
        char report[64];
        int status;
        
};

#endif //RANGE_SENSOR_H