#include <server/pipo_socket.h>

PipoSocket pipoSocket;
PipoSocket::PipoSocket() {
  this->ws = nullptr;
}
void PipoSocket::setup(AsyncWebSocket* ws) {
  this->ws = ws;
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
  if (ws == nullptr)
    return;
  unsigned long now = millis();
  // if (now - lastSendTime < 50) {
  //   iterations += 1;
  //   return;
  // }
  if (now - lastCleanTime > 1000) {
    ws->cleanupClients(1);
    lastCleanTime = now;
  }
  std::string message = "fps,";
  message += std::to_string((float)iterations);
  message += ",";
  message += std::to_string((float)now - lastSendTime);
  iterations = 0;
  lastSendTime = now;
  const auto& sensor_dat = input_sensor.get_sensor_dat_map();
  for (auto const& pair : sensor_dat) {
    string axis_name = pair.first;
    float sensor_val = input_sensor.get_value(axis_name);
    bool sensor_bool = input_sensor.get_bool_value(axis_name);
    float sensor_min = input_sensor.get_limit_min(axis_name);
    float sensor_max = input_sensor.get_limit_max(axis_name);

    // check if axis is enabled, outside deadzone and not disabled
    if (!input_sensor.test_outside_deadzone(axis_name))
      continue;

    message += "\nsensor";
    message += axis_name;
    message += ",";
    message += std::to_string(sensor_val);
    message += ",";
    message += std::to_string(sensor_bool);
  }
  ws->textAll(message.c_str());
  if (logs.hasNews()) {
    message = "logs,";
    message += logs.readLogs(true).c_str();
    ws->textAll(message.c_str());
  }
}
