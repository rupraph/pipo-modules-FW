#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"

// should test quat 9 and other inputs

void MotionSensor::init() {
#if defined(PROTO_ATOM)
  Wire.begin(2, 1, 400000);
#else
  Wire.begin(17, 18, 400000);
#endif

  log_i("init motion sensor");
  icm20948.init(icmSettings);
}

void MotionSensor::setup() {
  // Load last relative orientation reference
  if (!motiondata.begin("motion-store", false)) {
    log_e("Failed to open motion-store namespace");
    reference_set = false;
    return;
  }

  // Default to identity quaternion (no rotation) if not stored
  quat_ref_w = motiondata.getFloat("quat_ref_w", 1.0);
  quat_ref_x = motiondata.getFloat("quat_ref_x", 0.0);
  quat_ref_y = motiondata.getFloat("quat_ref_y", 0.0);
  quat_ref_z = motiondata.getFloat("quat_ref_z", 0.0);

  // Validate quaternion integrity - check if normalized
  float magnitude = sqrt(quat_ref_w * quat_ref_w + quat_ref_x * quat_ref_x +
                         quat_ref_y * quat_ref_y + quat_ref_z * quat_ref_z);
  bool is_identity = (quat_ref_w == 1.0 && quat_ref_x == 0.0 &&
                      quat_ref_y == 0.0 && quat_ref_z == 0.0);
  bool is_corrupted = (abs(magnitude - 1.0) > 0.1) && !is_identity;

  if (is_corrupted) {
    log_w("Corrupted quaternion data detected (magnitude: %.4f)", magnitude);
    log_w("Clearing corrupted data - will be set on first update");
    motiondata.clear();  // Clear all corrupted data
    quat_ref_w = 1.0;
    quat_ref_x = 0.0;
    quat_ref_y = 0.0;
    quat_ref_z = 0.0;
    reference_set = false;
  } else if (is_identity) {
    reference_set = false;
    log_i("No stored reference orientation - will be set on first update");
  } else {
    reference_set = true;
    log_i("Loaded stored reference orientation (magnitude: %.4f)", magnitude);
  }

  motiondata.end();  // Close namespace after reading

  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MotionSensor::toggle_relative_mode() {
  relative_mode = !relative_mode;
  log_i("Relative mode %s", relative_mode ? "ENABLED" : "DISABLED");
}

void MotionSensor::set_new_reference_orientation() {
  reset_reference_orientation();
  log_i("New reference orientation set");
}

// void MotionSensor::update() {
//   measure_sensor();
//   //should add step counter
//   process_sensor_neutral_filter();
//   process_sensor_triggers();

//   //Todo: this is not the best way to do the offset measurement. Should be updated when better task management is implemented
//   // each class should control its own update task ? (so that it can be paused)
//   if (!measure_offset_flag) {
//     for (auto const& pair : sensor_dat) {
//       sensor_dat[pair.first].value -= sensor_dat[pair.first].offset;
//     }
//   }

//   if (measure_offset_flag) {
//     measure_offset_counter++;
//     offset += sensor_dat[axis_to_measure_offset].value;
//     if (measure_offset_counter >= OFFSET_CAL_SAMPLES_NB) {
//       sensor_dat[axis_to_measure_offset].offset =
//           offset / OFFSET_CAL_SAMPLES_NB;
//       measure_offset_flag = false;
//       Serial.print("offset of ");
//       Serial.print(axis_to_measure_offset.c_str());
//       Serial.print(" is: ");
//       Serial.println(sensor_dat[axis_to_measure_offset].offset);
//     }
//   }
// }

bool MotionSensor::measure_sensor() {
  icm20948.task();
  bool data_ready = false;
  /////////  Read Quat6 orientation data
  if (relative_mode) {
    // Mode 1: Pure relative orientation using quaternion differential tracking
    if (icm20948.quat9DataIsReady()) {
      icm20948.readQuat9Data(&quat_w, &quat_x, &quat_y, &quat_z);
      if (!reference_set) {
        // Set initial orientation as reference and normalize
        quat_ref_w = quat_w;
        quat_ref_x = quat_x;
        quat_ref_y = quat_y;
        quat_ref_z = quat_z;
        normalize_quaternion(quat_ref_w, quat_ref_x, quat_ref_y, quat_ref_z);

        // Open namespace for writing
        if (!motiondata.begin("motion-store", false)) {
          log_e("Failed to open motion-store for writing");
        } else {
          // Write with error checking (putFloat returns 4 on success, 0 on failure)
          bool write_success = true;
          write_success &= (motiondata.putFloat("quat_ref_w", quat_ref_w) == 4);
          write_success &= (motiondata.putFloat("quat_ref_x", quat_ref_x) == 4);
          write_success &= (motiondata.putFloat("quat_ref_y", quat_ref_y) == 4);
          write_success &= (motiondata.putFloat("quat_ref_z", quat_ref_z) == 4);

          motiondata.end();  // Close namespace

          if (write_success) {
            reference_set = true;
            log_i("Reference orientation saved");
            log_i("Ref quat: w=%.4f x=%.4f y=%.4f z=%.4f", quat_ref_w,
                  quat_ref_x, quat_ref_y, quat_ref_z);
          } else {
            log_e("Failed to save reference orientation to NVS");
          }
        }
      }
      calc_differential_euler_angles();
      data_ready = true;
    }
  } else {
    // Mode 2: quat9 (absolute orientation)
    if (icm20948.quat9DataIsReady()) {
      icm20948.readQuat9Data(&quat_w, &quat_x, &quat_y, &quat_z);
      calc_euler_angles();
      data_ready = true;
    }
  }

  //////////  Read acceleration data
  if (icm20948.linearAccelDataIsReady()) {
    icm20948.readLinearAccelData(&sensor_dat["accX"].raw_value,
                                 &sensor_dat["accY"].raw_value,
                                 &sensor_dat["accZ"].raw_value);
    convert_accell();
    data_ready = true;
  }

  if (icm20948.magDataIsReady()) {
    icm20948.readMagData(&sensor_dat["magX"].raw_value,
                         &sensor_dat["magY"].raw_value,
                         &sensor_dat["magZ"].raw_value);
    sensor_dat["magX"].value = sensor_dat["magX"].raw_value;
    //     filter_map["magX"].process(sensor_dat["magX"].raw_value);
    sensor_dat["magY"].value = sensor_dat["magY"].raw_value;
    //     filter_map["magY"].process(sensor_dat["magY"].raw_value);
    sensor_dat["magZ"].value = sensor_dat["magZ"].raw_value;
    //     filter_map["magZ"].process(sensor_dat["magZ"].raw_value);
    // unit seems to be (mT)
    data_ready = true;
  }

  if (icm20948.gyroDataIsReady()) {
    icm20948.readGyroData(&sensor_dat["gyroX"].raw_value,
                          &sensor_dat["gyroY"].raw_value,
                          &sensor_dat["gyroZ"].raw_value);
    sensor_dat["gyroX"].value = sensor_dat["gyroX"].raw_value;
    //     filter_map["gyroX"].process(sensor_dat["gyroX"].raw_value);
    sensor_dat["gyroY"].value = sensor_dat["gyroY"].raw_value;
    //     filter_map["gyroY"].process(sensor_dat["gyroY"].raw_value);
    sensor_dat["gyroZ"].value = sensor_dat["gyroZ"].raw_value;
    //     filter_map["gyroZ"].process(sensor_dat["gyroZ"].raw_value);
    // unit is degrees per second (°/s)
    data_ready = true;
  }

  //activity recog
  // d = drive
  // w = walk
  // r = run
  // b = bike
  // s = still
  // t = tilt
  // icm20948.readHarData(&har);
  //Todo: try read additional data from sensor

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
    // Output the Quaternion data in the format expected by ZaneL's Node.js
    // Quaternion animation tool Serial.print(F("{\"quat_w\":"));
    // Serial.print(q0, 3);
    // Serial.print(F(", \"quat_x\":"));
    // Serial.print(q1, 3);
    // Serial.print(F(", \"quat_y\":"));
    // Serial.print(q2, 3);
    // Serial.print(F(", \"quat_z\":"));
    // Serial.print(q3, 3);
    // Serial.println(F("}"));
  }
  return data_ready;
}

void MotionSensor::calc_euler_angles() {
  //Todo: make order of euler angles configurable
  double siny_cosp = +2.0 * (quat_w * quat_z + quat_x * quat_y);
  double cosy_cosp = +1.0 - 2.0 * (quat_y * quat_y + quat_z * quat_z);
  sensor_dat["yaw"].raw_value = atan2(siny_cosp, cosy_cosp) * 180.0 / PI;

  sensor_dat["yaw"].value = sensor_dat["yaw"].raw_value;
  //     filter_map["yaw"].process(sensor_dat["yaw"].raw_value);

  // pitch (y-axis rotation)
  double sinp = +2.0 * (quat_w * quat_y - quat_z * quat_x);
  if (fabs(sinp) >= 1)
    sensor_dat["pitch"].raw_value =
        copysign(PI / 2, sinp) * 180.0 / PI;  // use 90 degrees if out of range
  else
    sensor_dat["pitch"].raw_value = asin(sinp) * 180.0 / PI;

  sensor_dat["pitch"].value = sensor_dat["pitch"].raw_value;
  //     filter_map["pitch"].process(sensor_dat["pitch"].raw_value);

  // roll (x-axis rotation)
  double sinr_cosp = +2.0 * (quat_w * quat_x + quat_y * quat_z);
  double cosr_cosp = +1.0 - 2.0 * (quat_x * quat_x + quat_y * quat_y);
  sensor_dat["roll"].raw_value = atan2(sinr_cosp, cosr_cosp) * 180.0 / PI;

  sensor_dat["roll"].value = sensor_dat["roll"].raw_value;
  //     filter_map["roll"].process(sensor_dat["roll"].raw_value);
}

void MotionSensor::calc_differential_euler_angles() {
  // Calculate relative quaternion: q_relative = q_reference_conjugate * q_current
  // This gives us the rotation FROM reference TO current orientation
  // Quaternion conjugate: q_ref_conj = [w, -x, -y, -z]
  // Quaternion multiplication: q1 * q2 = [w1*w2 - x1*x2 - y1*y2 - z1*z2,
  //                                       w1*x2 + x1*w2 + y1*z2 - z1*y2,
  //                                       w1*y2 - x1*z2 + y1*w2 + z1*x2,
  //                                       w1*z2 + x1*y2 - y1*x2 + z1*w2]

  // Normalize current quaternion
  float curr_w = quat_w, curr_x = quat_x, curr_y = quat_y, curr_z = quat_z;
  normalize_quaternion(curr_w, curr_x, curr_y, curr_z);

  // q_relative = q_ref_conjugate * q_current
  // where q_ref_conjugate = [quat_ref_w, -quat_ref_x, -quat_ref_y, -quat_ref_z]
  // explicit calulation:
  // float rel_w = quat_ref_w * curr_w - (-quat_ref_x) * curr_x -
  //               (-quat_ref_y) * curr_y - (-quat_ref_z) * curr_z;
  // float rel_x = quat_ref_w * curr_x + (-quat_ref_x) * curr_w +
  //               (-quat_ref_y) * curr_z - (-quat_ref_z) * curr_y;
  // float rel_y = quat_ref_w * curr_y - (-quat_ref_x) * curr_z +
  //               (-quat_ref_y) * curr_w + (-quat_ref_z) * curr_x;
  // float rel_z = quat_ref_w * curr_z + (-quat_ref_x) * curr_y -
  //               (-quat_ref_y) * curr_x + (-quat_ref_z) * curr_w;

  // Simplify (removing double negatives):
  rel_w = quat_ref_w * curr_w + quat_ref_x * curr_x + quat_ref_y * curr_y +
          quat_ref_z * curr_z;
  rel_x = quat_ref_w * curr_x - quat_ref_x * curr_w - quat_ref_y * curr_z +
          quat_ref_z * curr_y;
  rel_y = quat_ref_w * curr_y + quat_ref_x * curr_z - quat_ref_y * curr_w -
          quat_ref_z * curr_x;
  rel_z = quat_ref_w * curr_z - quat_ref_x * curr_y + quat_ref_y * curr_x -
          quat_ref_z * curr_w;

  // Convert relative quaternion to Euler angles
  // Yaw (z-axis rotation)
  double siny_cosp = +2.0 * (rel_w * rel_z + rel_x * rel_y);
  double cosy_cosp = +1.0 - 2.0 * (rel_y * rel_y + rel_z * rel_z);
  sensor_dat["yaw"].raw_value = atan2(siny_cosp, cosy_cosp) * 180.0 / PI;
  sensor_dat["yaw"].value = sensor_dat["yaw"].raw_value;

  // Pitch (y-axis rotation)
  double sinp = +2.0 * (rel_w * rel_y - rel_z * rel_x);
  if (fabs(sinp) >= 1)
    sensor_dat["pitch"].raw_value = copysign(PI / 2, sinp) * 180.0 / PI;
  else
    sensor_dat["pitch"].raw_value = asin(sinp) * 180.0 / PI;
  sensor_dat["pitch"].value = sensor_dat["pitch"].raw_value;

  // Roll (x-axis rotation)
  double sinr_cosp = +2.0 * (rel_w * rel_x + rel_y * rel_z);
  double cosr_cosp = +1.0 - 2.0 * (rel_x * rel_x + rel_y * rel_y);
  sensor_dat["roll"].raw_value = atan2(sinr_cosp, cosr_cosp) * 180.0 / PI;
  sensor_dat["roll"].value = sensor_dat["roll"].raw_value;
}

void MotionSensor::convert_accell() {
  //no conversion here
  sensor_dat["accX"].value = sensor_dat["accX"].raw_value;
  // filter_map["accX"].process(sensor_dat["accX"].raw_value);
  sensor_dat["accY"].value = sensor_dat["accY"].raw_value;
  // filter_map["accY"].process(sensor_dat["accY"].raw_value);
  sensor_dat["accZ"].value = sensor_dat["accZ"].raw_value;
  // filter_map["accZ"].process(sensor_dat["accZ"].raw_value);
}

void MotionSensor::set_sensor_config(JsonObject config, bool debug = false) {
  if (debug) {
    log_d("set_sensor_config");
  }
  if (config["relative_mode"].is<bool>()) {
    bool old_mode = relative_mode;
    relative_mode = config["relative_mode"];
    if (relative_mode && !old_mode) {
      // Switching to differential mode - reset reference
      reference_set = false;
    }
  }
  if (debug) {
    Serial.println(relative_mode);
    Serial.println("set_sensor_config_end");
  }
}

JsonDocument MotionSensor::get_sensor_config(bool debug = false) {
  JsonDocument config;
  config["relative_mode"] = relative_mode;
  return config;
}

void MotionSensor::reset_reference_orientation() {
  reference_set = false;
  Serial.println("Reference orientation will be reset on next update");
}

void MotionSensor::normalize_quaternion(float& w, float& x, float& y,
                                        float& z) {
  float norm = sqrt(w * w + x * x + y * y + z * z);
  if (norm > 0.0f) {
    w /= norm;
    x /= norm;
    y /= norm;
    z /= norm;
  }
}

// void MotionSensor::get_relative_quat(float& w, float& x, float& y, float& z) {
//   if (!reference_set) {
//     w = 1.0f;
//     x = 0.0f;
//     y = 0.0f;
//     z = 0.0f;
//     return;
//   }

//   // Normalize current quaternion
//   float curr_w = quat_w, curr_x = quat_x, curr_y = quat_y, curr_z = quat_z;
//   normalize_quaternion(curr_w, curr_x, curr_y, curr_z);

//   // Calculate relative quaternion: q_relative = q_ref_conjugate * q_current
//   w = quat_ref_w * curr_w + quat_ref_x * curr_x + quat_ref_y * curr_y +
//       quat_ref_z * curr_z;
//   x = quat_ref_w * curr_x - quat_ref_x * curr_w - quat_ref_y * curr_z +
//       quat_ref_z * curr_y;
//   y = quat_ref_w * curr_y + quat_ref_x * curr_z - quat_ref_y * curr_w -
//       quat_ref_z * curr_x;
//   z = quat_ref_w * curr_z - quat_ref_x * curr_y + quat_ref_y * curr_x -
//       quat_ref_z * curr_w;
// }

#endif  // PIPO_MOTION