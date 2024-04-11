#ifndef ACC_SENSOR_H
#define ACC_SENSOR_H

#include <ICM_20948.h>
#include <vector>

using namespace std;

class sensor
{   
    private:
        ICM_20948_I2C myICM;
        icm_20948_DMP_data_t data;
        double q1;
        double q2;
        double q3;
        double q0;
    public:
        sensor();
        void init();
        void setup();
        void update();
        void calc_euler_angles();
        bool initialized = false;
        bool enable_send_vizualizer = false;

        double roll;
        double pitch;
        double yaw;

        // add selection for mode (addel only, oriantation, ...)
};

#endif //ACC_SENSOR_H