#ifndef SENSOR_H
#define SENSOR_H

#include <VL53L0X.h>

class dist_Sensor {
    private:
        VL53L0X sensor;
        int sda;
        int scl;
        int sensorBuffer[20];
        int buffidx = 0;
    public:
        dist_Sensor(int sda, int scl);
        void init();
        void print_last();
        void update();
};

#endif //SENSOR_H