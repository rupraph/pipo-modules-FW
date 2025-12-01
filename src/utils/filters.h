#ifndef FILTERS_H
#define FILTERS_H

#include <math.h>
#include <Arduino.h>
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
    //deltaTime in seconds
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

//Todo: not toroughly tested for eg buffer resize...)
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

  void reset(float initialValue) {
    for (int i = 0; i < bufferSize; i++) {
      buffer[i] = initialValue;
    }
    sum = initialValue * bufferSize;
    bufferIndex = 0;
  }

  float process(float rawValue) {
    sum -= buffer[bufferIndex];
    buffer[bufferIndex] = rawValue;
    sum += buffer[bufferIndex];
    bufferIndex = (bufferIndex + 1) % bufferSize;
    return sum / bufferSize;
  }
};

//Exponential Moving Average filter
class EMAFilter {
 private:
  float alpha;  // Smoothing factor
  float prevValue;
  bool initialized;

 public:
  EMAFilter(float alpha = 0.1)
      : alpha(alpha), prevValue(0.0f), initialized(false) {}

  void setAlpha(float newAlpha) { alpha = newAlpha; }

  float process(float rawValue) {
    if (!initialized) {
      prevValue = rawValue;
      initialized = true;
    }
    float emaValue = alpha * rawValue + (1 - alpha) * prevValue;
    prevValue = emaValue;
    return emaValue;
  }
};

// this is a deadband filter. returns new value if it is outside of the deadband, else returns prevValue
// can work for creating a neutral band, but not good when moving
class DeadBandFilter {
 private:
  float deadband;
  float prevValue;

 public:
  DeadBandFilter(float deadband = 0.1) : deadband(deadband), prevValue(0.0f) {}

  void setDeadband(float newDeadband) { deadband = newDeadband; }

  float process(float rawValue) {
    if (abs(rawValue - prevValue) > deadband) {
      prevValue = rawValue;
    }
    return prevValue;
  }
};

//
class StateBasedFilter {
 private:
  float deadband;
  float prevValue;
  bool isMoving;
  int stableCount;
  int stableThreshold;

 public:
  StateBasedFilter(float deadband = 0.1, int stableThreshold = 10)
      : deadband(deadband),
        prevValue(0.0f),
        isMoving(false),
        stableCount(0),
        stableThreshold(stableThreshold) {}

  void setDeadband(float newDeadband) { deadband = newDeadband; }
  void setStableThreshold(int newStableThreshold) {
    stableThreshold = newStableThreshold;
  }

  float process(float rawValue) {
    if (deadband == 0) {
      return rawValue;
    }
    if (isMoving) {
      // If moving, follow the input closely
      prevValue = rawValue;
      // Check if the signal has stabilized
      if (abs(rawValue - prevValue) <= deadband) {
        stableCount++;
        if (stableCount >= stableThreshold) {
          isMoving = false;
          stableCount = 0;
        }
      } else {
        stableCount = 0;
      }
    } else {
      // If stable, apply deadband filter
      if (abs(rawValue - prevValue) > deadband) {
        prevValue = rawValue;
        isMoving = true;
      }
    }
    return prevValue;
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

// from Kalman Filter for Noise Reducer on Sensor Readings, Alfian Ma’arif a,1,*, Iswanto b,2, Aninditya Anggari Nuryono
// does not seem to work properly for now
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

class MedianFilter {
 private:
  std::vector<float> buffer;
  size_t windowSize;

 public:
  MedianFilter(size_t windowSize = 20) : windowSize(windowSize) {
    buffer.reserve(windowSize);
  }

  float process(float rawValue) {
    if (buffer.size() >= windowSize) {
      buffer.erase(buffer.begin());
    }
    buffer.push_back(rawValue);

    std::vector<float> sortedBuffer = buffer;
    std::sort(sortedBuffer.begin(), sortedBuffer.end());

    return sortedBuffer[sortedBuffer.size() / 2];
  }
};

#endif  //hp_filter_h