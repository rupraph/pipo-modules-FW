#include "dist_sensor.h"
#include <Wire.h>


dist_Sensor::dist_Sensor(int sda, int scl): sda(sda), scl(scl){
    }

void dist_Sensor::init() {
        Wire.begin(sda,scl);
        sensor.setTimeout(500);
        if (!sensor.init()) {
            Serial.println("Failed to detect and initialize sensor!");
            while (1) {}
        }
        sensor.startContinuous();
    }

void dist_Sensor::update() {
        sensorBuffer[buffidx] = sensor.readRangeContinuousMillimeters();
        buffidx = (buffidx + 1) % 20;
    }

void dist_Sensor::print_last() {
        Serial.println(sensorBuffer[buffidx]);
    }
