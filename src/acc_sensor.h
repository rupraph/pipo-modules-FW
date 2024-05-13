#ifndef ACC_SENSOR_H
#define ACC_SENSOR_H

#include <ICM_20948.h>
#include <vector>
#include <unordered_map>
#include "json.hpp"
#include "filters.h"

using namespace std;
using json = nlohmann::json;

//todo missing capital at sensor
class sensor
{   

    public:

        static sensor& getInstance() {
            static sensor instance; // This is the single instance of Sensor. It's static, which means it's shared by all instances of the class.
            return instance;
        }
        sensor(sensor const&) = delete;// This deletes the copy constructor. This means you can't create a new Sensor object by copying an existing one.
        void operator=(sensor const&) = delete;// This deletes the assignment operator. This means you can't assign one Sensor object to another.

        
        bool initialized = false;
        bool enable_send_vizualizer = false;


        // might want to move this structure outside of the class
        struct SensorDat {
            bool enabled;
            bool inverted; 
            int deadZone; // % of the total range
            float value;
            float offset;
        };

        unordered_map<string, SensorDat> sensor_dat = {
            {"roll", {false, false, 0, 0, 0}},
            {"pitch", {false, false, 0, 0, 0}},
            {"yaw", {false, false, 0, 0, 0}},
            {"accX", {true, false, 0, 0, 0}},
            {"accY", {false, false, 0, 0, 0}},
            {"accZ", {false, false, 0, 0, 0}}
        };


        void init();
        void setup();
        void update();
        void calc_euler_angles();

        //config 
        json get_config(bool debug=false);
        void set_config(json& config,bool debug=false);

        //Getter setters

        unordered_map<string, SensorDat> get_sensor_dat_map();

        bool get_enabled(const std::string& axis);
        void set_enabled(const std::string& axis, bool value);

        bool get_inverted(const std::string& axis);
        void set_inverted(const std::string& axis, bool value);

        int get_deadZone(const std::string& axis);
        void set_deadZone(const std::string& axis, int value);

        float get_value(const std::string& axis);
        void set_value(const std::string& axis, float value);

        float get_offset(const std::string& axis);
        void set_offset(const std::string& axis, float value);

        void convert_accell();

        void teleplot_data(string axis);



    private:

        //HighPassFilter hp_filter_accX; // seems like substracting lowpass is slightly better

        // use lowpass substraction to remove gravity. not optimal, but ok for basic processing.
        LowPassFilter lp_filter_accX;
        LowPassFilter lp_filter_accY;
        LowPassFilter lp_filter_accZ;


        const float acc_range=8.0; // carefull scale change is not miplemented in the sensor setup.
        float accel_scale_coef=acc_range/32767.0; // range here is bare +-8, 16, etc...  * 9.81;to convert in m/s-2


        sensor():lp_filter_accX(),lp_filter_accY(),lp_filter_accZ()
        {

        }; // This is the constructor. It's private, which means it can only be called from within the class.


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