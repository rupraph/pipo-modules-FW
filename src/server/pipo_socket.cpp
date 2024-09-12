#include <server/pipo_socket.h>

PipoSocket pipoSocket;
PipoSocket::PipoSocket() {
  this->ws = nullptr;
  this->input_sens = nullptr;
}
void PipoSocket::setup(AsyncWebSocket *ws, Sensor *sensor) {
  this->ws = ws;
  this->input_sens = sensor;
}
void PipoSocket::sendNoteOn(int note, int velocity, int channel) {
  if (ws == nullptr)
    return;
  String msg = "noteon";
  msg += channel;
  msg += ",";
  msg += note;
  msg += ",";
  msg += velocity;
  ws->textAll(msg.c_str());
}
void PipoSocket::sendNoteOff(int note, int velocity, int channel) {
  if (ws == nullptr)
    return;
  String msg = "noteoff";
  msg += channel;
  msg += ",";
  msg += note;
  msg += ",";
  msg += velocity;
  ws->textAll(msg.c_str());
}

void PipoSocket::sendSensorValue(std::string axis, float value) {
  if (ws == nullptr)
    return;
  String msg = "sensor";
  msg += axis.c_str();
  msg += ",";
  msg += value;
  ws->textAll(msg.c_str());
}

void PipoSocket::loop() {
  if (ws == nullptr || input_sens == nullptr)
    return;
  unsigned long now = millis();
  if (now - lastSendTime < 50) {
    iterations += 1;
    return;
  }
  std::string message = "fps,";
  message += std::to_string((float)iterations);
  message += ",";
  message += std::to_string((float)now - lastSendTime);
  iterations = 0;
  lastSendTime = now;
  const auto &sensor_dat = input_sens->get_sensor_dat_map();
  for (auto const &pair : sensor_dat) {
    string axis_name = pair.first;
    float sensor_val = input_sens->get_value(axis_name);
    float sensor_min = input_sens->get_limit_min(axis_name);
    float sensor_max = input_sens->get_limit_max(axis_name);

    // check if axis is enabled, outside deadzone and not disabled
    if (!input_sens->get_enabled(axis_name) ||
        !input_sens->test_outside_deadzone(axis_name))
      continue;

    message += "\nsensor";
    message += axis_name;
    message += ",";
    message += std::to_string(sensor_val);
  }
  ws->textAll(message.c_str());
  if (logs.hasNews()) {
    message = "logs,";
    message += logs.readLogs(true).c_str();
    ws->textAll(message.c_str());
  }
}
