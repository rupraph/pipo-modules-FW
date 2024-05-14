#include "range_sensor.h"


void RangeSensor::init()
{
    vl53l4cx.setI2cDevice(&Wire);
    //vl53l4cx.setXShutPin(4); Todo
    vl53l4cx.begin();
    vl53l4cx.VL53L4CX_Off();
    if(vl53l4cx.InitSensor(0x12)==VL53L4CX_ERROR_NONE){
        Serial.println("VL53L4CX sensor found and initialized");
    }else{
        Serial.println("VL53L4CX sensor not found or not initialized");
    }
}

void RangeSensor::setup()
{
    vl53l4cx.VL53L4CX_StartMeasurement();
    vl53l4cx.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(20000);
    NewDataReady = 0;
    no_of_object_found = 0;
}

void RangeSensor::update()
{
    int j;
    do {
        status = vl53l4cx.VL53L4CX_GetMeasurementDataReady(&NewDataReady);
    } while (!NewDataReady);

    if ((!status) && (NewDataReady != 0)) {
        status = vl53l4cx.VL53L4CX_GetMultiRangingData(pMultiRangingData);
        no_of_object_found = pMultiRangingData->NumberOfObjectsFound;

        // get first object distance
        sensor_dat["dist"].value = pMultiRangingData->RangeData[0].RangeMilliMeter;

        Serial.print(">VL53L4CX-0:");
        Serial.print(sensor_dat["dist"].value);
        Serial.println();

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
}


// generic functions /////////////////


