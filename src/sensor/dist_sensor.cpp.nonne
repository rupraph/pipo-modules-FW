#include "dist_sensor.h"
#include <Wire.h>


dist_Sensor::dist_Sensor(int sda, int scl): sda(sda), scl(scl){
    }

void dist_Sensor::init() {
        Wire.begin(sda,scl,100000);
        sensor.setTimeout(1000);
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

int dist_Sensor::get_current() {
        return sensorBuffer[buffidx];
    }

int dist_Sensor::get_moving_average(int window_size){
    if (window_size > BUFFER_SIZE) { // Assuming bufferSize is the size of your buffer
        // If x is greater than the size of the buffer, 
        // we can't calculate the moving average of the last x values.
        return 0.0;
    }

    double sum = 0.0;
    for (int i = 0; i < window_size; i++) {
        // Calculate the correct index taking into account the circular nature of the buffer
        int idx = (buffidx - i - 1 + bufferSize) % bufferSize;
        sum += sensorBuffer[idx];
    }

    return int(sum / window_size);
}