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
    OutputData& out = output_map[i];
    if (out.pindir == PinMode::OUT) {
      if (out.out_mode == SERVO) {
        // Serial.print("Servo ");
        // Serial.print(i);
        // Serial.print(" ");
        // Serial.print(out.value);
        // Serial.print("pin ");
        // Serial.println(pin_map[i]);
        pwm.writeServo(pin_map[i], out.value);
        // out.servo.write(16, out.value);
      } else if (out.out_mode == PWM) {
        pwm.write(pin_map[i], out.value * 255);
      }
    }
  }
}

void AnalogOut::set_value(string name, float value) {
  // Serial.println("Setting servo");
  if (name.substr(0, 1) != "A") {
    Serial.print("can't set value to analog out, wrong name ");
    Serial.println(name.c_str());
    return;
  }
  int index = get_index_from_name(name);
  output_map[index].value = value;
}

void AnalogOut::set_config(JsonObject config) {
  // Serial.println("Setting config");
  Serial.println("Setting sensor config");
  serializeJsonPretty(config, Serial);
  for (size_t i = 0; i < 6; i++) {
    string key = "A0" + to_string(i + 1);
    // Serial.println(key.c_str());
    if (config[key].is<JsonObject>()) {
      // Serial.print("key");
      // Serial.print(key.c_str());
      // Serial.print(" ");
      // Serial.println(config[key]["pindir"].as<bool>());
      output_map[i].out_mode = config[key]["outmode"];
      set_pin_dir(i, config[key]["pindir"].as<bool>());
      //Todo: setter for changing out mode

    } else {
      Serial.println("key not found");
    }
  }
}

JsonDocument AnalogOut::get_config() {
  // Serial.println("Getting config");
  JsonDocument obj;
  for (size_t i = 0; i < 6; i++) {
    string key = "A0" + to_string(i + 1);
    obj[key]["pindir"] = output_map[i].pindir;
    obj[key]["outmode"] = output_map[i].out_mode;
    obj[key]["lmax"] = output_map[i].lmax;
    obj[key]["lmin"] = output_map[i].lmin;
  }
  return obj;
}

void AnalogOut::set_pin_dir(int index, bool dir) {
  output_map[index].pindir = dir;
  if (dir) {
    if (output_map[index].out_mode == SERVO) {
      // Seems like attach for servo causes servo not to work
      // Serial.print("Attaching servo ");
      // Serial.print(output_map[index].name.c_str());
      // Serial.print(" on pin ");
      // Serial.println(pin_map[index]);

      // output_map[index].servo.attach(pin_map[index]);
    }
    // Todo: add pwm mode there
  } else {
    if (output_map[index].out_mode == SERVO) {
      // output_map[index].servo.detach(pin_map[index]);
    }
  }
}

//Todo: set

bool AnalogOut::get_pin_dir(int index) {
  return output_map[index].pindir;
}

bool AnalogOut::get_pin_dir(string name) {

  int position = get_index_from_name(name);
  // Serial.print(name.c_str());
  // Serial.print(" ");
  // Serial.println(position);

  return output_map[position].pindir;
}

int AnalogOut::get_index_from_name(string name) {
  // This wont work above 9
  return stoi(name.substr(2)) - 1;
}

#endif