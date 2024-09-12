#include "acc_sensor.h"

// should test quat 9 and other inputs

void MotionSensor::init() {
#if defined(PROTO_ATOM)
  Wire.begin(2, 1, 400000);
#else
  Wire.begin(17, 18, 400000);
#endif

  Serial.println("init motion sensor");
  icm20948.init(icmSettings);
}

void MotionSensor::setup() {
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

void MotionSensor::update() {
  icm20948.task();

  /////////  Read Quat6 orientation data
  if (icm20948.quat6DataIsReady()) {
    icm20948.readQuat6Data(&quat_w, &quat_x, &quat_y, &quat_z);
    calc_euler_angles();
  }

  // send to adafruit visualizer
  if (enable_send_vizualizer) {
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

  //////////  Read acceleration data
  if (icm20948.linearAccelDataIsReady()) {
    icm20948.readLinearAccelData(&raw_accX, &raw_accY, &raw_accZ);
    convert_accell();
  }

  ////////// Read euler from lib
  //icm20948.readEuler6Data(&roll, &pitch, &yaw);

  //icm20948.readEuler9Data(&roll, &pitch, &yaw);

  ////////// Read activity recognition
  // if (icm20948.harDataIsReady())
  // {
  //     icm20948.readHarData(&activity);
  // }

  ////////// Read step counter
  // if (icm20948.stepsDataIsReady())
  // {
  //     icm20948.readStepsData(&steps);
  // }
}

void MotionSensor::calc_euler_angles() {
  // // opposite order roll pitch yaw
  // // roll (x-axis rotation)
  // double sinr_cosp = +2.0 * (q0 * q1 + q2 * q3);
  // double cosr_cosp = +1.0 - 2.0 * (q1 * q1 + q2 * q2);
  // sensor_dat["roll"].value = atan2(sinr_cosp, cosr_cosp) * 180.0 / PI;
  // // pitch (y-axis rotation)
  // double sinp = +2.0 * (q0 * q2 - q3 * q1);
  // if (fabs(sinp) >= 1)
  //     sensor_dat["pitch"].value = copysign(PI / 2, sinp) * 180.0 / PI; // use 90 degrees if out of range
  // else
  //     sensor_dat["pitch"].value = asin(sinp) * 180.0 / PI;
  // // yaw (z-axis rotation)
  // double siny_cosp = +2.0 * (q0 * q3 + q1 * q2);
  // double cosy_cosp = +1.0 - 2.0 * (q2 * q2 + q3 * q3);
  // sensor_dat["yaw"].value = atan2(siny_cosp, cosy_cosp) * 180.0 / PI;

  // attempt to get roll as outer axis, and yaw as inner.
  // Yaw, Pitch, Roll (ZYX): (copilot says first is inner)
  // yaw (z-axis rotation)
  double siny_cosp = +2.0 * (quat_w * quat_z + quat_x * quat_y);
  double cosy_cosp = +1.0 - 2.0 * (quat_y * quat_y + quat_z * quat_z);
  sensor_dat["yaw"].value = atan2(siny_cosp, cosy_cosp) * 180.0 / PI;

  // pitch (y-axis rotation)
  double sinp = +2.0 * (quat_w * quat_y - quat_z * quat_x);
  if (fabs(sinp) >= 1)
    sensor_dat["pitch"].value =
        copysign(PI / 2, sinp) * 180.0 / PI;  // use 90 degrees if out of range
  else
    sensor_dat["pitch"].value = asin(sinp) * 180.0 / PI;

  // roll (x-axis rotation)
  double sinr_cosp = +2.0 * (quat_w * quat_x + quat_y * quat_z);
  double cosr_cosp = +1.0 - 2.0 * (quat_x * quat_x + quat_y * quat_y);
  sensor_dat["roll"].value = atan2(sinr_cosp, cosr_cosp) * 180.0 / PI;

  ////original order
  //// roll (x-axis rotation)
  //   double t0 = +2.0 * (q0 * q1 + q2 * q3);
  //   double t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
  //   sensor_dat["roll"].value = atan2(t0, t1) * 180.0 / PI;
  //// pitch (y-axis rotation)
  //   double t2 = +2.0 * (q0 * q2 - q3 * q1);
  //   t2 = t2 > 1.0 ? 1.0 : t2;
  //   t2 = t2 < -1.0 ? -1.0 : t2;
  //   sensor_dat["pitch"].value = asin(t2) * 180.0 / PI;
  //// yaw (z-axis rotation)
  //   double t3 = +2.0 * (q0 * q3 + q1 * q2);
  //   double t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
  //   sensor_dat["yaw"].value = atan2(t3, t4) * 180.0 / PI;

  //   //adafruit style visualizer  // carefull with the order that the viewer expect... !!
  // Serial.print("Orientation: ");
  // Serial.print(sensor_dat["yaw"].value);
  // Serial.print(", ");
  // Serial.print(sensor_dat["pitch"].value);
  // Serial.print(", ");
  // Serial.println(sensor_dat["roll"].value);
}

void MotionSensor::convert_accell() {
  sensor_dat["accX"].value = raw_accX;  //* accel_scale_coef;
  sensor_dat["accY"].value = raw_accY;  //* accel_scale_coef;
  sensor_dat["accZ"].value = raw_accZ;  //* accel_scale_coef;
}