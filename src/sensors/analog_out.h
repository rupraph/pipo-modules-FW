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

// Trying if enum are more practical than unordered in this case
enum PinMode { IN, OUT };
enum OutputMode { PWM, SERVO };
enum AnalogOutput { A01, A02, A03, A04, A05, A06 };

//will not use for first tests
struct OutputData {
  string name;
  bool pindir;  // 0 input 1 output
  int out_mode;
  float value;
  float lmax;
  float lmin;
  Servo servo = Servo();
  Pwm pwm = Pwm();
};

class AnalogOut {

 public:
  void setup();
  void set_config(JsonObject config);
  JsonDocument get_config();
  void update();
  // void set_pwm(int pin, int value);
  void set_value(string name, float value);

  void set_pin_dir(int index, bool dir);
  void set_out_mode(int index, int mode);

  bool get_pin_dir(int index);
  bool get_pin_dir(string name);

  int get_index_from_name(string name);

  OutputData output_map[6];

 private:
  int pin_map[6] = {16, 15, 14, 13, 12, 11};
};

extern AnalogOut analog_out;

#endif