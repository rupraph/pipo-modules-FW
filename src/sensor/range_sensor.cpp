#include "range_sensor.h"

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
  //delay(10);
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
  lp_filter.set_cutoffFrequency(5.0);
  ma_filter = MovingAverageFilter(3);
  km_filter = KalmanFilter(1, 1);
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

void RangeSensor::update() {
  start_duration();
  int j;

  // do {
  status = vl53l4cx.VL53L4CX_GetMeasurementDataReady(&NewDataReady);
  // } while (!NewDataReady);

  if ((!status) && (NewDataReady != 0)) {
    status = vl53l4cx.VL53L4CX_GetMultiRangingData(pMultiRangingData);

    // Todo
    // ADD ambient light
    // float ambiant = pMultiRangingData->AmbiantPerSpad;

    no_of_object_found = pMultiRangingData->NumberOfObjectsFound;

    //Todo: deal with second object detected causing wrong distance report.
    // get first object distance
    // ignore negative values
    float dist = pMultiRangingData->RangeData[0].RangeMilliMeter;

    if (dist < 0 || !pMultiRangingData->RangeData[0].RangeStatus ==
                        VL53L4CX_RANGESTATUS_RANGE_VALID) {

    }
    // not sure if capping is optimal to be there in sensor or better in engine/translators
    else {
      if (dist > sensor_dat["dist"].limit_max) {

        within_range = false;
        sensor_dat["dist"].value = sensor_dat["dist"].limit_max;
      } else {
        within_range = true;
        sensor_dat["dist"].value = ma_filter.process(lp_filter.process(
            dist));  //Todo: Not working good with notes yet need fo cc
                     //sensor_dat["dist"].value = km_filter.process(dist);
                     //sensor_dat["dist"].value = dist;
      }
      if (within_range_prev == false && within_range == true) {
        sensor_dat["dist"].triggered = true;
      } else {
        //move reset when sending the note ???
        sensor_dat["dist"].triggered = false;
      }
      within_range_prev = within_range;
    }

    // Serial.print(">VL53L4CX-0:");
    // Serial.print(sensor_dat["dist"].value);
    // Serial.println();

    // check for other detected objects
    // for (j = 0; j < no_of_object_found; j++) {
    // if (j != 0) {
    //     Serial.print(">VL53L4CX-:");
    //     Serial.print(pMultiRangingData->RangeData[j].RangeMilliMeter);
    //     Serial.println();
    // }
    // }
    if (status == 0) {
      status = vl53l4cx.VL53L4CX_ClearInterruptAndStartMeasurement();
    }
  }
  end_duration();
  measured_loop_duration();
}

// generic functions /////////////////
