#ifdef PIPO_MGC3130

#include "sensors/mgc3130_sensor.h"

void MGC3130Sensor::init() {
  Wire.begin(1, 7);  // SDA=1, SCL=7
}

void MGC3130Sensor::setup() {
  while (!mgc3130.begin()) {
    Serial.println(
        "begin error! Please check whether the connection is correct");
    delay(100);
  };
  Serial.println("begin success!!!");
  while (mgc3130.enableApproachDetection() != 0) {
    delay(100);
  }

  Serial.println("config success!!!");
}

bool MGC3130Sensor::measure_sensor() {
  mgc3130.sensorDataRecv();
  if (mgc3130.havePositionInfo()) {
    sensor_dat["x"].raw_value = mgc3130.getPositionX();
    sensor_dat["y"].raw_value = mgc3130.getPositionY();
    sensor_dat["z"].raw_value = mgc3130.getPositionZ();
    return true;
  }
  return false;
}

void MGC3130Sensor::set_sensor_config(JsonObject config, bool debug) {
  // not implemented for now
}

JsonDocument MGC3130Sensor::get_sensor_config(bool debug) {
  StaticJsonDocument<256> doc;
  // not implemented for now
  return doc;
}

#endif
