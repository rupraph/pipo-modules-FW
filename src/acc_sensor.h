#ifndef ACC_SENSOR_H
#define ACC_SENSOR_H

#include <ICM_20948.h>
#include <vector>
#include <unordered_map>

using namespace std;

class sensor
{   

    public:

        static sensor& getInstance() {
            static sensor instance; // This is the single instance of Sensor. It's static, which means it's shared by all instances of the class.
            return instance;
        }
        sensor(sensor const&) = delete;// This deletes the copy constructor. This means you can't create a new Sensor object by copying an existing one.
        void operator=(sensor const&) = delete;// This deletes the assignment operator. This means you can't assign one Sensor object to another.

        void init();
        void setup();
        void update();
        void calc_euler_angles();
        bool initialized = false;
        bool enable_send_vizualizer = false;

        double roll;
        double pitch;
        double yaw;

        unordered_map<string, bool> data_map= {
            {"roll", true},
            {"pitch", true},
            {"yaw",true},
            {"accX",false},
            {"accY",false},
            {"accZ",false}
            };

        // add selection for mode (addel only, oriantation, ...)


    private:
        sensor(){}; // This is the constructor. It's private, which means it can only be called from within the class.
        ICM_20948_I2C myICM;
        icm_20948_DMP_data_t data;
        double q1;
        double q2;
        double q3;
        double q0;

};

#endif //ACC_SENSOR_H