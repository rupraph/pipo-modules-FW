#ifndef SENSOR_H
#define SENSOR_H

#include <VL53L0X.h>

#define BUFFER_SIZE 20

class dist_Sensor {
    private:
        VL53L0X sensor;
        int sda;
        int scl;
        int const bufferSize = 20;
        int sensorBuffer[BUFFER_SIZE];
        int buffidx = 0;
    public:
        dist_Sensor(int sda, int scl);
        void init();    
        void print_last();
        void update();
        int get_current();
        int get_moving_average(int window_size);
};

#endif //SENSOR_H