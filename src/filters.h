#ifndef FILTERS_H
#define FILTERS_H

#include <math.h>

class HighPassFilter {
    private:
        float prevRawValue, prevHighPassValue;
        float alpha;
        float cutoffFrequency;

    public:
        HighPassFilter(float cutoffFrequency) {
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
    float cutoffFrequency;

public:
    LowPassFilter(float cutoffFrequency) {
        this->cutoffFrequency = cutoffFrequency;
        prevValue = 0;
    }

    float process(float rawValue, float deltaTime) {
        alpha = deltaTime / (deltaTime + 1/(2 * M_PI * cutoffFrequency));
        float lowPassValue = (1 - alpha) * prevValue + alpha * rawValue;
        prevValue = lowPassValue;
        return lowPassValue;
    }
};

#endif //hp_filter_h