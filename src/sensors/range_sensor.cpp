#ifdef PIPO_RANGE

#include "sensors/range_sensor.h"

void RangeSensor::init() {
#if defined(PROTO_ATOM)
  Wire.begin(2, 1, 400000);
#else
  Wire.begin(17, 18, 400000);
#endif

  vl53l4cx.setI2cDevice(&Wire);
  vl53l4cx.setXShutPin(15);
  vl53l4cx.begin();
  vl53l4cx.VL53L4CX_Off();
  VL53L4CX_Error initstatus = vl53l4cx.InitSensor(0x12);
  if (initstatus == VL53L4CX_ERROR_NONE) {
    Serial.println("VL53L4CX sensor found and initialized");
  } else {
    Serial.println("VL53L4CX sensor not found or not initialized");
  }
}

void RangeSensor::setup() {
  vl53l4cx.VL53L4CX_StartMeasurement();
  vl53l4cx.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(20000);
  NewDataReady = 0;
  no_of_object_found = 0;

  // use of filter should likely a t one point be configurable
  lp_filter.set_cutoffFrequency(18.0);
  ma_filter = MovingAverageFilter(3);
  km_filter = KalmanFilter(1, 1);
#ifdef DEBUG_HEAP
  pipoDebugHeap();
#endif
}

void RangeSensor::update() {
  start_duration();
  int j;

  status = vl53l4cx.VL53L4CX_GetMeasurementDataReady(&NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    status = vl53l4cx.VL53L4CX_GetMultiRangingData(pMultiRangingData);
    // Todo: add ambient light capture
    // float ambiant = pMultiRangingData->AmbiantPerSpad;
    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
    //Todo: deal with second object detected
    // -> test driver to report single value. tested working on another sensor from same type.
    sensor_dat["dist"].raw_value =
        pMultiRangingData->RangeData[0].RangeMilliMeter / 10.0;

    // process result
    // when out of range
    if (sensor_dat["dist"].raw_value < 0 ||
        !pMultiRangingData->RangeData[0].RangeStatus ==
            VL53L4CX_RANGESTATUS_RANGE_VALID) {
      if (!hold_mode) {
        sensor_dat["dist"].value_prev = sensor_dat["dist"].value;
        sensor_dat["dist"].value = sensor_dat["dist"].lmax;
      }

    }
    // not sure if capping is optimal to be here in sensor class or better in engine/translators
    // when in range
    else {

      if ((hold_mode &&
           sensor_dat["dist"].raw_value < sensor_dat["dist"].lmax) ||
          !hold_mode) {
        sensor_dat["dist"].value_prev = sensor_dat["dist"].value;
        sensor_dat["dist"].value =
            ma_filter.process(sensor_dat["dist"].raw_value);
      }
    }

    //  ma_filter.process(lp_filter.process(dist));

    //Todo: optimize filter choices
    //sensor_dat["dist"].value = km_filter.process(dist);
    process_sensor_neutral_filter();
    process_sensor_triggers();
  }
  if (status == 0) {
    status = vl53l4cx.VL53L4CX_ClearInterruptAndStartMeasurement();
  }
  end_duration();
  measured_loop_duration();
}

void RangeSensor::set_sensor_config(JsonObject config, bool debug) {
  if (debug) {
    Serial.println("set_sensor_config");
  }
  if (config["hold_mode"].is<bool>()) {
    hold_mode = config["hold_mode"];
  }
  if (debug) {
    Serial.println(hold_mode);
    Serial.println("set_sensor_config_end");
  }
}

JsonDocument RangeSensor::get_sensor_config(bool debug) {
  JsonDocument config;
  config["hold_mode"] = hold_mode;
  return config;
}

#endif