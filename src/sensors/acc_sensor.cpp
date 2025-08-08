#ifdef PIPO_MOTION
#include "sensors/acc_sensor.h"

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
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void MotionSensor::update() {
  icm20948.task();
  /////////  Read Quat6 orientation data
  if (relative_mode) {
    // Mode 1: Pure relative orientation using quaternion differential tracking
    if (icm20948.quat6DataIsReady()) {
      icm20948.readQuat6Data(&quat_w, &quat_x, &quat_y, &quat_z);
      if (!reference_set) {
        // Set initial orientation as reference
        quat_ref_w = quat_w;
        quat_ref_x = quat_x;
        quat_ref_y = quat_y;
        quat_ref_z = quat_z;
        reference_set = true;
        Serial.println("Reference orientation set");
      }
      calc_differential_euler_angles();
    }
  } else {
    // Mode 2: quat9 (absolute orientation)
    if (icm20948.quat9DataIsReady()) {
      icm20948.readQuat9Data(&quat_w, &quat_x, &quat_y, &quat_z);
      calc_euler_angles();
    }
  }

  //////////  Read acceleration data
  if (icm20948.linearAccelDataIsReady()) {
    icm20948.readLinearAccelData(&sensor_dat["accX"].raw_value,
                                 &sensor_dat["accY"].raw_value,
                                 &sensor_dat["accZ"].raw_value);
    convert_accell();
  }

  if (icm20948.magDataIsReady()) {
    icm20948.readMagData(&sensor_dat["magX"].raw_value,
                         &sensor_dat["magY"].raw_value,
                         &sensor_dat["magZ"].raw_value);
    sensor_dat["magX"].value = sensor_dat["magX"].raw_value;
    //     filter_map["magX"].process(sensor_dat["magX"].raw_value);
    sensor_dat["magY"].value = sensor_dat["magX"].raw_value;
    //     filter_map["magY"].process(sensor_dat["magX"].raw_value);
    sensor_dat["magZ"].value = sensor_dat["magZ"].raw_value;
    //     filter_map["magZ"].process(sensor_dat["magZ"].raw_value);
    // unit seems to be (mT)
  }

  //activity recog
  // d = drive
  // w = walk
  // r = run
  // b = bike
  // s = still
  // t = tilt
  // icm20948.readHarData(&har);

  //should add step counter
  process_sensor_neutral_filter();
  process_sensor_triggers();

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

  //Todo: this is not the best way to do the offset measurement. Should be updated when better task management is implemented
  // each class should control its own update task ? (so that it can be paused)
  if (!measure_offset_flag) {
    for (auto const& pair : sensor_dat) {
      sensor_dat[pair.first].value -= sensor_dat[pair.first].offset;
    }
  }

  if (measure_offset_flag) {
    measure_offset_counter++;
    offset += sensor_dat[axis_to_measure_offset].value;
    if (measure_offset_counter >= OFFSET_CAL_SAMPLES_NB) {
      sensor_dat[axis_to_measure_offset].offset =
          offset / OFFSET_CAL_SAMPLES_NB;
      measure_offset_flag = false;
      Serial.print("offset of ");
      Serial.print(axis_to_measure_offset.c_str());
      Serial.print(" is: ");
      Serial.println(sensor_dat[axis_to_measure_offset].offset);
    }
  }
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
  // Calculate relative quaternion: q_relative = q_current * q_reference_inverse
  // Quaternion inverse: q_inv = [w, -x, -y, -z] / |q|^2
  // For unit quaternions, |q|^2 = 1, so q_inv = [w, -x, -y, -z]

  // q_relative = q_current * q_ref_inverse
  float rel_w = quat_w * quat_ref_w + quat_x * quat_ref_x +
                quat_y * quat_ref_y + quat_z * quat_ref_z;
  float rel_x = quat_w * (-quat_ref_x) + quat_x * quat_ref_w +
                quat_y * (-quat_ref_z) + quat_z * quat_ref_y;
  float rel_y = quat_w * (-quat_ref_y) + quat_x * quat_ref_z +
                quat_y * quat_ref_w + quat_z * (-quat_ref_x);
  float rel_z = quat_w * (-quat_ref_z) + quat_x * (-quat_ref_y) +
                quat_y * quat_ref_x + quat_z * quat_ref_w;

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

void MotionSensor::measure_offset(const string& axis_name) {
  if (!measure_offset_flag) {
    measure_offset_flag = true;
    axis_to_measure_offset = axis_name;
    measure_offset_counter = 0;
    Serial.println("start offset measurement");
  }
}

void MotionSensor::set_sensor_config(JsonObject config, bool debug = false) {
  if (debug) {
    Serial.println("set_sensor_config");
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

#endif  // PIPO_MOTION