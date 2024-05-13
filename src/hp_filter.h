#ifndef HP_FILTER_H
#define HP_FILTER_H

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

#endif //hp_filter_h