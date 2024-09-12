#include "acc_sensor.h"

// should test quat 9 and other inputs

void MotionSensor::init()
{
    #if defined(PROTO_ATOM)
        Wire.begin(2, 1, 400000);
    #else
        Wire.begin(17, 18, 400000);
    #endif
    
    Serial.println("init motion sensor");
    icm20948.init(icmSettings);
}


void MotionSensor::setup()
{
    #ifdef DEBUG_HEAP
        Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
    #endif
}

void MotionSensor::update()
{
    icm20948.task();
    /////////  Read Quat6 orientation data
    if (icm20948.quat6DataIsReady())
    {
        icm20948.readQuat6Data(&quat_w, &quat_x, &quat_y, &quat_z);
        calc_euler_angles();
    }

    //////////  Read acceleration data
    if (icm20948.linearAccelDataIsReady())
    {
        icm20948.readLinearAccelData(&raw_accX, &raw_accY, &raw_accZ);
        convert_accell();
    }

    process_sensor_triggers();
    

    //Todo: try read additional data from sensor

    // send to adafruit visualizer 
    if (enable_send_vizualizer)
    {
        // Serial.print("Quaternion:");
        // Serial.print(q0);
        // Serial.print(",");
        // Serial.print(q1);
        // Serial.print(",");
        // Serial.print(q2);
        // Serial.print(",");
        // Serial.println(q3);
        // Output the Quaternion data in the format expected by ZaneL's Node.js Quaternion animation tool
        // Serial.print(F("{\"quat_w\":"));
        // Serial.print(q0, 3);
        // Serial.print(F(", \"quat_x\":"));
        // Serial.print(q1, 3);
        // Serial.print(F(", \"quat_y\":"));
        // Serial.print(q2, 3);
        // Serial.print(F(", \"quat_z\":"));
        // Serial.print(q3, 3);
        // Serial.println(F("}"));
    }

}


void MotionSensor::calc_euler_angles()
{     
    //Todo: make order of euler angles configurable
    double siny_cosp = +2.0 * (quat_w * quat_z + quat_x * quat_y);
    double cosy_cosp = +1.0 - 2.0 * (quat_y * quat_y + quat_z * quat_z);
    sensor_dat["yaw"].value = atan2(siny_cosp, cosy_cosp) * 180.0 / PI;

    // pitch (y-axis rotation)
    double sinp = +2.0 * (quat_w * quat_y - quat_z * quat_x);
    if (fabs(sinp) >= 1)
        sensor_dat["pitch"].value = copysign(PI / 2, sinp) * 180.0 / PI; // use 90 degrees if out of range
    else
        sensor_dat["pitch"].value = asin(sinp) * 180.0 / PI;

    // roll (x-axis rotation)
    double sinr_cosp = +2.0 * (quat_w * quat_x + quat_y * quat_z);
    double cosr_cosp = +1.0 - 2.0 * (quat_x * quat_x + quat_y * quat_y);
    sensor_dat["roll"].value = atan2(sinr_cosp, cosr_cosp) * 180.0 / PI;    
}

void MotionSensor::convert_accell()
{    
    sensor_dat["accX"].value = raw_accX; 
    sensor_dat["accY"].value = raw_accY; 
    sensor_dat["accZ"].value = raw_accZ; 
}