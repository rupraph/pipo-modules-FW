#include "sensor.h"
#include <Wire.h>
#include <VL53L0X.h>


VL53L0X sensor;

const int sda=13;
const int scl=14;
int sensorBuffer[20];
int buffidx = 0;

void sensorSetup(int sda, int scl) {
    Wire.begin(sda, scl);
    sensor.setTimeout(100);
    if (!sensor.init())
    {
        Serial.println("Failed to detect and initialize sensor!");
        while (1) {}
    }
    sensor.startContinuous();
}

void sensorupdate() {
    //sensorBuffer[index] = sensor.readRangeContinuousMillimeters();
    //index++;
}

//     // if (index >= sizeof(sensorBuffer)) {
//     //     index = 0;}
    