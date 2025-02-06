#ifndef ANALOG_OUT_H
#define ANALOG_OUT_H

#include <Arduino.h>
#include "HW_CONFIG.h"
#include <unordered_map>
#include <Servo.h>
#include <ArduinoJson.h>

//This class will handle output data to the hardware. For now, lets have PWM, servo. then PDM ?
// I should likley do like for sensor, having a higher output clas, with specifics children for each output type

using namespace std;

//will not use for first tests
struct OutputData {
  string name;
  int pinmode;
  int out_mode;
  float value;
  float lmax;
  float lmin;
  Servo servo;
};

// Trying if enum are more practical than unordered in this case
enum PinMode { IN, OUT };
enum OutputMode { PWM, SERVO };
enum AnalogOutput { A01, A02, A03, A04, A05, A06 };

class AnalogOut {

 public:
  void setup();
  void set_config(JsonObject config);
  JsonDocument get_config();
  void update();
  // void set_pwm(int pin, int value);
  void set_value(string name, float value);

  void set_pin_mode(int index, int mode);
  void set_out_mode(int index, int mode);

  bool get_pin_mode(int index);
  bool get_pin_mode(string name);

  OutputData output_map[6];

 private:
  int pin_map[6] = {16, 15, 14, 13, 12, 11};
};

extern AnalogOut analog_out;

#endif