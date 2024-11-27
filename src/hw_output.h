#ifndef hw_output_h
#define hw_output_h

#include <Arduino.h>
#include "HW_CONFIG.h"

//This class will handle output data to the hardware. For now, lets have PWM, and servo

//will not use for first tests

class HwOutput {

  //should likely hav a map with pin, mode, value

 public:
  void update();
  void set_pwm(int pin, int value);
  void set_servo(int pin, int value);
};

extern HwOutput hw_output;

#endif