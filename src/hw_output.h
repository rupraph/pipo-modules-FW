#ifndef hw_output_h
#define hw_output_h

#include <Arduino.h>
#include "HW_CONFIG.h"
#include "utils/config.h"
#include <unordered_map>
#include "sensors/sensors.h"

#ifdef PIPO_ANALOG
#include <Servo.h>
#endif

//This class will handle output data to the hardware. For now, lets have PWM, servo. then PDM ?
// I should likley do like for sensor, having a higher output clas, with specifics children for each output type

//will not use for first tests
struct OutputData {

  int pin;
  int mode;
  float value;
};

// Trying if enum are more practical than unordered in this case
enum OutputMode { PWM, SERVO };
enum AnalogOutput { A01, A02, A03, A04, A05, A06 };

class HwOutput {

  //should likely hav a map with pin, mode, value

 public:
  HwOutput() {
    output_map[A01] = {16, SERVO, 0};
    output_map[A02] = {15, SERVO, 0};
    output_map[A03] = {14, SERVO, 0};
    output_map[A04] = {13, SERVO, 0};
    output_map[A05] = {12, SERVO, 0};
    output_map[A06] = {11, SERVO, 0};
  }
  void setup();
  void update();
  // void set_pwm(int pin, int value);
  void set_servo(int pin, float value);

 private:
  OutputData output_map[6];
#ifdef PIPO_ANALOG
  Servo servo1 = Servo();
  Servo servo2 = Servo();
  Servo servo3 = Servo();
  Servo servo4 = Servo();
  Servo servo5 = Servo();
  Servo servo6 = Servo();
#endif
};

extern HwOutput hw_output;

#endif