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
  //   void update() override;
  bool measure_sensor() override;
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;
  void measure_offset(const std::string& axis_name) override;
  //   void measure_offset_all();

  void calc_euler_angles();
  void convert_accell();

  void get_quat(float& w, float& x, float& y, float& z) {
    w = quat_w;
    x = quat_x;
    y = quat_y;
    z = quat_z;
  }

  bool enable_send_vizualizer =
      false;  //set on/off serial messages for vizualizer

 private:
  //config
  bool relative_mode =
      true;  //uses quat6 or quat9 (6 = relative to start, 9 = absolute - corection to north is slow)

  //theses filters are for noise reduction.
  unordered_map<string, EMAFilter> filter_map = {
      {"roll", EMAFilter(0.7)}, {"pitch", EMAFilter(0.7)},
      {"yaw", EMAFilter(0.7)},  {"accX", EMAFilter(0.7)},
      {"accY", EMAFilter(0.7)}, {"accZ", EMAFilter(0.7)},
      {"magX", EMAFilter(0.7)}, {"magY", EMAFilter(0.7)},
      {"magZ", EMAFilter(0.7)}};

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
          true,  // Enables magnetometer output // Enables quaternion output
      .enable_gravity = false,            // Enables gravity vector output
      .enable_linearAcceleration = true,  // Enables linear acceleration output
      .enable_quaternion6 = true,         // Enables quaternion 6DOF output
      .enable_quaternion9 = true,         // Enables quaternion 9DOF output
      .enable_har = false,                // Enables activity recognition
      .enable_steps = false,              // Enables step counter
      .gyroscope_frequency = 1,      // Max frequency = 225, min frequency = 1
      .accelerometer_frequency = 1,  // Max frequency = 225, min frequency = 1
      .magnetometer_frequency = 60,  // Max frequency = 70, min frequency = 1
      .gravity_frequency = 1,        // Max frequency = 225, min frequency = 1
      .linearAcceleration_frequency =
          225,                       // Max frequency = 225, min frequency = 1
      .quaternion6_frequency = 100,  // Max frequency = 225, min frequency = 50
      .quaternion9_frequency = 100,  // Max frequency = 225, min frequency = 50
      .har_frequency = 50,           // Max frequency = 225, min frequency = 50
      .steps_frequency = 50          // Max frequency = 225, min frequency = 50
  };

  float quat_w;
  float quat_x;
  float quat_y;
  float quat_z;

  // holds raw data from sensor
  //   float raw_accX;
  //   float raw_accY;
  //   float raw_accZ;

  //   float raw_magX;
  //   float raw_magY;
  //   float raw_magZ;

  bool measure_offset_flag = false;
  string axis_to_measure_offset;
  int measure_offset_counter = 0;
  float offset = 0;
};

#endif  //ACC_SENSOR_H