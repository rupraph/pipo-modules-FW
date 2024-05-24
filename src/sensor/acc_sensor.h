#ifndef ACC_SENSOR_H
#define ACC_SENSOR_H

#include "sensor/input_sensor.h"
#include <ICM_20948.h>
#include "HW_CONFIG.h"


class MotionSensor: public Sensor{   
    public:
        MotionSensor():lp_filter_accX(),lp_filter_accY(),lp_filter_accZ()
        {
            sensor_dat = {
            {"roll", {false, false,     0,  0,  0,  90.0,   false}},
            {"pitch", {false, false,    0,  0,  0,  180.0,  false}},
            {"yaw", {false, false,      0,  0,  0,  180.0,  false}},
            {"accX", {true, false,      0.1,0,  0,  8.0,    false}},
            {"accY", {false, false,     0,  0,  0,  8.0,    false}},
            {"accZ", {false, false,     0,  0,  0,  8.0,    false}}
        };

        }; 

        void init() override;
        void setup() override;
        void update() override;

        bool initialized = false;
        bool enable_send_vizualizer = false;

        void calc_euler_angles();

        void convert_accell();


    private:

        // for taking orientation reference. test for now, might not keep this solution
        bool firstCall = true;
        double initialRoll, initialPitch;

        //HighPassFilter hp_filter_accX; // seems like substracting lowpass is slightly better

        // use lowpass substraction to remove gravity from accel. not optimal, but ok for basic processing.
        LowPassFilter lp_filter_accX;
        LowPassFilter lp_filter_accY;
        LowPassFilter lp_filter_accZ;

        // acellerometer value convertion
        const float acc_range=8.0; // full scale change. only for conversion, not linked/implemented with the sensor setup yet
        float accel_scale_coef=acc_range/32767.0; // range here is bare +-8, 16, etc...  * 9.81;to convert in m/s-2


        ICM_20948_I2C myICM;
        icm_20948_DMP_data_t data;
        double q1;
        double q2;
        double q3;
        double q0;

        // holds raw data from sensor
        float raw_accX;
        float raw_accY;
        float raw_accZ;

        //holds intermediate acceleration data
        float accX_t;
        float accY_t;
        float accZ_t;

        float speedX;

        //float hp_accX;

        float raw_gyroX;
        float raw_gyroY;
        float raw_gyroZ;

        unsigned long last_time;
        
};

#endif //ACC_SENSOR_H