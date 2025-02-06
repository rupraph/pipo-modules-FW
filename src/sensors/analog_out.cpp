#ifdef PIPO_ANALOG

#include "sensors/analog_out.h"

AnalogOut analog_out;

// LedC controller used for controlling the leds conflicts with the servo library

void AnalogOut::setup() {
  // Serial.println("Setting up HW output");
}

void AnalogOut::update() {
  // Serial.println("Updating HW output");
  // Serial.println(output_map[A01].value * 180.0f);
  // data ok. but not sure if lib does not work, or if hw was nok.
  for (size_t i = 0; i < 6; i++) {
    OutputData out = output_map[i];
    if (out.pinmode == PinMode::OUT) {
      if (out.out_mode == SERVO) {
        out.servo.write(pin_map[i], output_map[i].value * 180.0f);
      }
    }
  }
}

void AnalogOut::set_value(string name, float value) {
  // Serial.println("Setting servo");
  for (size_t i = 0; i < 6; i++) {
    if (output_map[i].name == name) {
      output_map[i].value = value;
    }
  }
}

void AnalogOut::set_config(JsonObject config) {
  // Serial.println("Setting config");
  for (size_t i = 0; i < 6; i++) {
    string key = "A0" + to_string(i + 1);
    Serial.println(key.c_str());
    if (config[key].is<JsonObject>()) {
      set_pin_mode(i, config[key]["pinmode"]);
      //Todo: setter for changing out mode
      output_map[i].out_mode = config[key]["outmode"];
    } else {
      Serial.println("key not found");
    }
  }
}

JsonDocument AnalogOut::get_config() {
  // Serial.println("Getting config");
  for (size_t i = 0; i < 6; i++) {
    JsonObject obj;
    obj["pinmode"] = output_map[i].pinmode;
    obj["outmode"] = output_map[i].out_mode;
    obj["lmax"] = output_map[i].lmax;
    obj["lmin"] = output_map[i].lmin;
  }
}

void AnalogOut::set_pin_mode(int index, PinMode mode) {
  output_map[index].pinmode = mode;
  if (mode == PinMode::OUT) {
    if (output_map[index].out_mode == SERVO) {
      output_map[index].servo.attach(pin_map[index]);
    }
    // Todo: add pwm mode there
  } else {
    if (output_map[index].out_mode == SERVO) {
      output_map[index].servo.detach(pin_map[index]);
    }
  }
}

//Todo: set

bool AnalogOut::get_pin_mode(int index) {
  return output_map[index].pinmode;
}

bool AnalogOut::get_pin_mode(string name) {

  int position = stoi(name.substr(1)) - 1;
  // Serial.print(name.c_str());
  // Serial.print(" ");
  // Serial.println(position);

  return output_map[position].pinmode;
}

#endif