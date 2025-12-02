#ifdef PIPO_RANGE

#include "sensors/pipo_range_sensor.h"

void PipoRangeSensor::init() {
#if defined(PROTO_ATOM)
  Wire.begin(2, 1, 400000);
#else
  Wire.begin(17, 18, 400000);
#endif

#if HW_REV == 10
  vl53l4cx.setI2cDevice(&Wire);
  vl53l4cx.setXShutPin(15);
  vl53l4cx.begin();
  vl53l4cx.VL53L4CX_Off();
  VL53L4CX_Error initstatus = vl53l4cx.InitSensor(0x12);
  if (initstatus == VL53L4CX_ERROR_NONE) {
    log_i("VL53L4CX sensor found and initialized");
  } else {
    log_w("VL53L4CX sensor not found or not initialized");
  }
#elif HW_REV >= 11
  vl53l1 = new VL53L1(&Wire, 15);
  vl53l1->begin();
  vl53l1->VL53L1_Off();
  VL53L1_Error initstatus = vl53l1->InitSensor(0x52);
  // delay(10);
  if (initstatus == VL53L1_ERROR_NONE) {
    log_i("VL53L1 sensor found and initialized");
  } else {
    log_w("VL53L1 sensor not found or not initialized");
  }
#endif
}

void PipoRangeSensor::setup() {
#if HW_REV == 10
  vl53l4cx.VL53L4CX_StartMeasurement();
  vl53l4cx.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(20000);
#elif HW_REV >= 11
  vl53l1->VL53L1_StartMeasurement();
  vl53l1->VL53L1_SetPresetMode(VL53L1_PRESETMODE_RANGING);
  vl53l1->VL53L1_SetMeasurementTimingBudgetMicroSeconds(
      12000);  //if not specified. 33ms. 16000 normal.
  vl53l1->VL53L1_ClearInterruptAndStartMeasurement();
#endif

  NewDataReady = 0;
  no_of_object_found = 0;

  // use of filter should likely a t one point be configurable
  // lp_filter.set_cutoffFrequency(18.0);
  ma_filter = MovingAverageFilter(3);
  // km_filter = KalmanFilter(1, 1);

  sensor_dat["dist"].NeutralFilter.setDeadband(0.1);
  sensor_dat["dist"].NeutralFilter.setStableThreshold(0.5);

  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void PipoRangeSensor::toggle_hold_mode() {
  hold_mode = !hold_mode;
  log_i("Hold mode %s", hold_mode ? "ENABLED" : "DISABLED");
}

bool PipoRangeSensor::measure_sensor() {
  // start_duration();
  int j;
  bool data_ready = false;

  // Store previous range state
  within_range_prev = within_range;

#if HW_REV == 10
  status = vl53l4cx.VL53L4CX_GetMeasurementDataReady(&NewDataReady);
#elif HW_REV >= 11
  status = vl53l1->VL53L1_GetMeasurementDataReady(&NewDataReady);
#endif

  if ((!status) && (NewDataReady != 0)) {
#if HW_REV == 10
    status = vl53l4cx.VL53L4CX_GetMultiRangingData(pMultiRangingData);
#elif HW_REV >= 11
    status = vl53l1->VL53L1_GetMultiRangingData(pMultiRangingData);
#endif
    // Todo: add ambient light capture
    // float ambiant = pMultiRangingData->AmbiantPerSpad;
    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;
    //Todo: deal with second object detected
    // -> test driver to report single value. tested working on another sensor from same type.
    sensor_dat["dist"].raw_value =
        pMultiRangingData->RangeData[0].RangeMilliMeter / 10.0;

    // process result
    // when out of range
    int8_t range_status =
#if HW_REV == 10
        range_status = pMultiRangingData->RangeData[0].RangeStatus ==
                       VL53L4CX_RANGESTATUS_RANGE_VALID;
#elif HW_REV >= 11
        range_status = pMultiRangingData->RangeData[0].RangeStatus ==
                       VL53L1_RANGESTATUS_RANGE_VALID;

#endif

    if (sensor_dat["dist"].raw_value < 0 || !range_status) {
      within_range = false;
      if (!hold_mode) {
        // sensor_dat["dist"].value_prev = sensor_dat["dist"].value;
        sensor_dat["dist"].value = abs_max;
        data_ready = true;
      }
      // Serial.println("Out of range");

    }
    // not sure if capping is optimal to be here in sensor class or better in engine/translators
    // when in range
    else {
      within_range = true;

      // Reset filter when transitioning into valid range to prevent spurious values
      if (within_range && !within_range_prev) {
        ma_filter.reset(sensor_dat["dist"].raw_value);
      }

      if ((hold_mode &&
           sensor_dat["dist"].raw_value < sensor_dat["dist"].lmax) ||
          !hold_mode) {
        // sensor_dat["dist"].value_prev = sensor_dat["dist"].value;
        // interval.stop();
        // interval.report();
        // interval.start();
        sensor_dat["dist"].value =
            ma_filter.process(sensor_dat["dist"].raw_value);
        data_ready = true;
      }

      //  ma_filter.process(lp_filter.process(dist));

      //Todo: optimize filter choices
      //sensor_dat["dist"].value = km_filter.process(dist);
      // process_sensor_neutral_filter();
      // process_sensor_triggers();
    }
    if (status == 0) {
#if HW_REV == 10
      status = vl53l4cx.VL53L4CX_ClearInterruptAndStartMeasurement();
#elif HW_REV >= 11
      status = vl53l1->VL53L1_ClearInterruptAndStartMeasurement();
#endif
    }
  }
  return data_ready;
  // end_duration();
  // measured_loop_duration();
}

// void PipoRangeSensor::measure_offset(const string& axis_name) {
//   sensor_dat[axis_name].offset = sensor_dat[axis_name].raw_value;
//   Serial.print("offset for ");
//   Serial.print(axis_name.c_str());
//   Serial.print(" : ");
//   Serial.println(sensor_dat[axis_name].offset);
// }

void RangeSensor::set_sensor_config(JsonObject config, bool debug) {
  if (debug) {
    log_d("set_sensor_config");
  }
  if (config["hold_mode"].is<bool>()) {
    set_hold_mode(config["hold_mode"].as<bool>());
  }
  if (debug) {
    log_d("%d", hold_mode);
    log_d("set_sensor_config_end");
  }
}

JsonDocument PipoRangeSensor::get_sensor_config(bool debug) {
  JsonDocument config;
  config["hold_mode"] = hold_mode;
  return config;
}

#endif