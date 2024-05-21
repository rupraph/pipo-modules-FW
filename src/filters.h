#ifndef FILTERS_H
#define FILTERS_H

#include <math.h>

class HighPassFilter {
    private:
        float prevRawValue, prevHighPassValue;
        float alpha;
        float cutoffFrequency=1.0;

    public:
        HighPassFilter() {
            this->cutoffFrequency = cutoffFrequency;
            prevRawValue = 0;
            prevHighPassValue = 0;
        }

        float process(float rawValue, float deltaTime) {
            alpha = deltaTime / (deltaTime + 1/(2 * M_PI * cutoffFrequency));
            float highPassValue = alpha * (prevHighPassValue + rawValue - prevRawValue);
            prevRawValue = rawValue;
            prevHighPassValue = highPassValue;
            return highPassValue;
        }
};


class LowPassFilter {
private:
    float prevValue;
    float alpha;
    float cutoffFrequency=1.0;
    unsigned long last_time = 0;
    float deltaTime = 0;

public:
    LowPassFilter() {
        // this->cutoffFrequency = cutoffFrequency;
        prevValue = 0;
    }

    void set_cutoffFrequency(float cutoffFrequency) {
        this->cutoffFrequency = cutoffFrequency;
    }

    float process(float rawValue, float deltaTimeExt=0) {
        //deltaTime in seconds
        if (deltaTimeExt != 0) {
            deltaTime = deltaTimeExt;
        } 
        else {
            deltaTime = (micros() - last_time)/1000000.0;
            last_time = micros();
        }

        alpha = deltaTime / (deltaTime + 1/(2 * M_PI * cutoffFrequency));
        float lowPassValue = (1 - alpha) * prevValue + alpha * rawValue;
        prevValue = lowPassValue;
        
        return lowPassValue;
        
    }
};


#endif //hp_filter_h