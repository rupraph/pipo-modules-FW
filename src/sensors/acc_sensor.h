#ifndef ACC_SENSOR_H
#define ACC_SENSOR_H

#include "sensors/input_sensor.h"
#include "HW_CONFIG.h"
#include "Arduino-ICM20948.h"
#include "utils/filters.h"
#include "utils/debug.h"
#include <Preferences.h>

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
    sensor_dat["gyroX"] = SensorDat();
    sensor_dat["gyroY"] = SensorDat();
    sensor_dat["gyroZ"] = SensorDat();
    sensor_dat["AccComb"] = SensorDat();
  };

  void init() override;
  void setup() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

  void calc_euler_angles();
  void convert_accell();
  void reset_reference_orientation();
  void calc_differential_euler_angles();
  void normalize_quaternion(float& w, float& x, float& y, float& z);

  void get_quat(float& w, float& x, float& y, float& z) {
    w = quat_w;
    x = quat_x;
    y = quat_y;
    z = quat_z;
  }
  void get_relative_quat(float& w, float& x, float& y, float& z) {
    w = rel_w;
    x = rel_x;
    y = rel_y;
    z = rel_z;
  }

  bool enable_send_vizualizer =
      false;  //set on/off serial messages for vizualizer

  // Button action methods
  void toggle_relative_mode();
  void set_new_reference_orientation();
  bool get_relative_mode() const { return relative_mode; }

 private:
  //config
  bool relative_mode =
      true;  //true = differential tracking (pure relative), false = quat9 absolute orientation

  //theses filters are for noise reduction.
  unordered_map<string, EMAFilter> filter_map = {
      {"roll", EMAFilter(0.7)},  {"pitch", EMAFilter(0.7)},
      {"yaw", EMAFilter(0.7)},   {"accX", EMAFilter(0.7)},
      {"accY", EMAFilter(0.7)},  {"accZ", EMAFilter(0.7)},
      {"magX", EMAFilter(0.7)},  {"magY", EMAFilter(0.7)},
      {"magZ", EMAFilter(0.7)},  {"gyroX", EMAFilter(0.7)},
      {"gyroY", EMAFilter(0.7)}, {"gyroZ", EMAFilter(0.7)}};

  ArduinoICM20948 icm20948;
  ArduinoICM20948Settings icmSettings = {
      .i2c_speed = 400000,   // i2c clock speed
      .is_SPI = false,       // Enable SPI, if disable use i2c
      .cs_pin = 10,          // SPI chip select pin
      .spi_speed = 7000000,  // SPI clock speed in Hz, max speed is 7MHz
      .mode = 1,             // 0 = low power mode, 1 = high performance mode
      .enable_gyroscope = true,      // Enables gyroscope output
      .enable_accelerometer = true,  // Enables accelerometer output
      .enable_magnetometer =
          true,  // Enables magnetometer output // Enables quaternion output
      .enable_gravity = false,            // Enables gravity vector output
      .enable_linearAcceleration = true,  // Enables linear acceleration output
      .enable_quaternion6 = true,         // Enables quaternion 6DOF output
      .enable_quaternion9 = true,         // Enables quaternion 9DOF output
      .enable_har = false,                // Enables activity recognition
      .enable_steps = false,              // Enables step counter
      .gyroscope_frequency = 200,    // Max frequency = 225, min frequency = 1
      .accelerometer_frequency = 1,  // Max frequency = 225, min frequency = 1
      .magnetometer_frequency = 60,  // Max frequency = 70, min frequency = 1
      .gravity_frequency = 1,        // Max frequency = 225, min frequency = 1
      .linearAcceleration_frequency =
          200,                       // Max frequency = 225, min frequency = 1
      .quaternion6_frequency = 100,  // Max frequency = 225, min frequency = 50
      .quaternion9_frequency = 100,  // Max frequency = 225, min frequency = 50
      .har_frequency = 50,           // Max frequency = 225, min frequency = 50
      .steps_frequency = 50          // Max frequency = 225, min frequency = 50
  };

  float quat_w;
  float quat_x;
  float quat_y;
  float quat_z;

  float rel_w;
  float rel_x;
  float rel_y;
  float rel_z;

  // Reference quaternion for differential tracking
  float quat_ref_w = 1.0;
  float quat_ref_x = 0.0;
  float quat_ref_y = 0.0;
  float quat_ref_z = 0.0;
  bool reference_set = false;

  Preferences motiondata;
};

#endif  //ACC_SENSOR_H