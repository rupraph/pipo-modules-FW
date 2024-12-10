#include "hw_output.h"

HwOutput hw_output;

// LedC controller used for controlling the leds conflicts with the servo library

void HwOutput::setup() {

#ifdef PIPO_ANALOG
  servo1.attach(output_map[A01].pin);
  servo2.attach(output_map[A02].pin);
  servo3.attach(output_map[A03].pin);
  servo4.attach(output_map[A04].pin);
  servo5.attach(output_map[A05].pin);
  servo6.attach(output_map[A06].pin);

#endif
  // Serial.println("Setting up HW output");
}

void HwOutput::update() {
// Serial.println("Updating HW output");
#ifdef PIPO_ANALOG
  Serial.println(output_map[A01].value * 180.0f);
  // data ok. but not sure if lib does not work, or if hw was nok.
  servo1.write(output_map[A01].pin, output_map[A01].value * 180.0f);
  servo2.write(output_map[A02].pin, output_map[A02].value * 180.0f);
  servo3.write(output_map[A03].pin, output_map[A03].value * 180.0f);
  servo4.write(output_map[A04].pin, output_map[A04].value * 180.0f);
  servo5.write(output_map[A05].pin, output_map[A05].value * 180.0f);
  servo6.write(output_map[A06].pin, output_map[A06].value * 180.0f);
#endif
}

void HwOutput::set_servo(int pin, float value) {
  // Serial.println("Setting servo");
  switch (pin) {
    case 0:
      output_map[A01].value = value;
      break;
    case 1:
      output_map[A02].value = value;
      break;
    case 2:
      output_map[A03].value = value;
      break;
    case 3:
      output_map[A04].value = value;
      break;
    case 4:
      output_map[A05].value = value;
      break;
    case 5:
      output_map[A06].value = value;
      break;
    default:
      Serial.println("Servo pin not found");
      break;
  }
}
