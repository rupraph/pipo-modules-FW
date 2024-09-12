#ifndef FILTERS_H
#define FILTERS_H

#include <Arduino.h>
#include <math.h>
#include <vector>

class HighPassFilter {
 private:
  float prevRawValue, prevHighPassValue;
  float alpha;
  float cutoffFrequency = 1.0;

 public:
  HighPassFilter() {
    this->cutoffFrequency = cutoffFrequency;
    prevRawValue = 0;
    prevHighPassValue = 0;
  }

  float process(float rawValue, float deltaTime) {
    alpha = deltaTime / (deltaTime + 1 / (2 * M_PI * cutoffFrequency));
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
  float cutoffFrequency = 1.0;
  unsigned long last_time = 0;
  float deltaTime = 0;

 public:
  LowPassFilter(float cutoffFrequency = 1.0) {
    // this->cutoffFrequency = cutoffFrequency;
    prevValue = 0;
  }

  void set_cutoffFrequency(float cutoffFrequency) {
    this->cutoffFrequency = cutoffFrequency;
  }

  float process(float rawValue, float deltaTimeExt = 0) {
    // deltaTime in seconds
    if (deltaTimeExt != 0) {
      deltaTime = deltaTimeExt;
    } else {
      deltaTime = (micros() - last_time) / 1000000.0;
      last_time = micros();
    }

    alpha = deltaTime / (deltaTime + 1 / (2 * M_PI * cutoffFrequency));
    float lowPassValue = (1 - alpha) * prevValue + alpha * rawValue;
    prevValue = lowPassValue;

    return lowPassValue;
  }
};

// Todo: not toroughly tested for eg buffer resize...)
class MovingAverageFilter {
 private:
  std::vector<float> buffer;
  int bufferSize;
  int bufferIndex;
  float sum;

 public:
  MovingAverageFilter(int bufferSize = 5)
      : buffer(bufferSize, 0.0f),
        bufferSize(bufferSize),
        bufferIndex(0),
        sum(0.0f) {}

  void setBufferSize(int newBufferSize) {
    bufferSize = newBufferSize;
    buffer.clear();
    buffer.resize(bufferSize, 0.0f);
    bufferIndex = 0;
    sum = 0.0f;
  }

  float process(float rawValue) {
    sum -= buffer[bufferIndex];
    buffer[bufferIndex] = rawValue;
    sum += buffer[bufferIndex];
    bufferIndex = (bufferIndex + 1) % bufferSize;
    return sum / bufferSize;
  }
};

// To be tested (copilot proposal)
// class KalmannFilter {
//     private:
//         float prevEstimate, prevError;
//         float processNoise, measurementNoise;
//     public:
//         KalmannFilter(float processNoise, float measurementNoise) {
//             this->processNoise = processNoise;
//             this->measurementNoise = measurementNoise;
//             prevEstimate = 0;
//             prevError = 0;
//         }

//         float process(float measurement) {
//             float estimate = prevEstimate;
//             float error = prevError + processNoise;
//             float kalmanGain = error / (error + measurementNoise);
//             estimate = estimate + kalmanGain * (measurement - estimate);
//             error = (1 - kalmanGain) * error;
//             prevEstimate = estimate;
//             prevError = error;
//             return estimate;
//         }
// };

// from Kalman Filter for Noise Reducer on Sensor Readings, Alfian Ma’arif
// a,1,*, Iswanto b,2, Aninditya Anggari Nuryono does not seem to work properly
// for now
class KalmanFilter {
 private:
  float current_dat, Kalman_dat;
  float Xt, Xt_update, Xt_prev;
  float Pt, Pt_update, Pt_prev;
  float Kt, R, Q;

 public:
  KalmanFilter(float R = 1, float Q = 1) {
    this->R = R;
    this->Q = Q;
    Pt_prev = 1;
  }

  float process(float measurement) {
    // Prediction
    Xt_update = Xt_prev;
    Pt_update = Pt_prev + Q;

    // Update
    Kt = Pt_update / (Pt_update + R);
    Xt = Xt_update + (Kt * (current_dat - Xt_update));
    Pt = (1 - Kt) * Pt_update;

    Xt_prev = Xt;
    Pt_prev = Pt;

    return Xt;
  }
};

#endif  // hp_filter_h