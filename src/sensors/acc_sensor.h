#ifndef ACC_SENSOR_H
#define ACC_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "Arduino-ICM20948.h"
#include "utils/filters.h"
#include "utils/debug.h"

class MotionSensor : public Sensor {
 public:
  MotionSensor() {
    sensor_dat["accX"] = SensorDat();
    sensor_dat["accY"] = SensorDat();
    sensor_dat["accZ"] = SensorDat();
    sensor_dat["roll"] = SensorDat();
    sensor_dat["pitch"] = SensorDat();
    sensor_dat["yaw"] = SensorDat();
    sensor_dat["magX"] = SensorDat();
    sensor_dat["magY"] = SensorDat();
    sensor_dat["magZ"] = SensorDat();
  };

  void init() override;
  void setup() override;
  void update() override;

  void calc_euler_angles();
  void convert_accell();

  bool enable_send_vizualizer =
      false;  //set on/off serial messages for vizualizer

 private:
  unordered_map<string, LowPassFilter> lp_filter_map = {
      {"roll", LowPassFilter(10)},
      {"pitch", LowPassFilter(10)},
      {"yaw", LowPassFilter(10)},
  };

  ArduinoICM20948 icm20948;
  ArduinoICM20948Settings icmSettings = {
      .i2c_speed = 400000,   // i2c clock speed
      .is_SPI = false,       // Enable SPI, if disable use i2c
      .cs_pin = 10,          // SPI chip select pin
      .spi_speed = 7000000,  // SPI clock speed in Hz, max speed is 7MHz
      .mode = 1,             // 0 = low power mode, 1 = high performance mode
      .enable_gyroscope = false,     // Enables gyroscope output
      .enable_accelerometer = true,  // Enables accelerometer output
      .enable_magnetometer =
          false,  // Enables magnetometer output // Enables quaternion output
      .enable_gravity = false,            // Enables gravity vector output
      .enable_linearAcceleration = true,  // Enables linear acceleration output
      .enable_quaternion6 = true,         // Enables quaternion 6DOF output
      .enable_quaternion9 = false,        // Enables quaternion 9DOF output
      .enable_har = false,                // Enables activity recognition
      .enable_steps = false,              // Enables step counter
      .gyroscope_frequency = 1,      // Max frequency = 225, min frequency = 1
      .accelerometer_frequency = 1,  // Max frequency = 225, min frequency = 1
      .magnetometer_frequency = 1,   // Max frequency = 70, min frequency = 1
      .gravity_frequency = 1,        // Max frequency = 225, min frequency = 1
      .linearAcceleration_frequency =
          225,                       // Max frequency = 225, min frequency = 1
      .quaternion6_frequency = 100,  // Max frequency = 225, min frequency = 50
      .quaternion9_frequency = 50,   // Max frequency = 225, min frequency = 50
      .har_frequency = 50,           // Max frequency = 225, min frequency = 50
      .steps_frequency = 50          // Max frequency = 225, min frequency = 50
  };

  float quat_w;
  float quat_x;
  float quat_y;
  float quat_z;

  // holds raw data from sensor
  float raw_accX;
  float raw_accY;
  float raw_accZ;

  float raw_magX;
  float raw_magY;
  float raw_magZ;
};

#endif  //ACC_SENSOR_H