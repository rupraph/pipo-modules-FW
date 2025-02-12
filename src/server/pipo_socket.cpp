#include <server/pipo_socket.h>

PipoSocket pipoSocket;
PipoSocket::PipoSocket() {
  this->ws = nullptr;
}
void PipoSocket::setup() {
  this->ws->onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client,
                        AwsEventType type, void* arg, uint8_t* data,
                        size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("Client connected: ID = %u, URL = %s\n", client->id(),
                    server->url());
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("Client disconnected: ID = %u, URL = %s\n", client->id(),
                    server->url());
    } else if (type == WS_EVT_ERROR) {
      uint16_t errorCode = *((uint16_t*)arg);
      Serial.printf(
          "WebSocket error: URL = %s, Client ID = %u, Error Code = %d, Data = "
          "%s\n",
          server->url(), client->id(), errorCode, (char*)data);
    } else if (type == WS_EVT_PONG) {
      Serial.printf("Pong received: URL = %s, Client ID = %u, Data = %s\n",
                    server->url(), client->id(),
                    (len) ? (char*)data : "No Data");
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo* info = (AwsFrameInfo*)arg;

      if (info->index == 0 && !info->final) {
        // Start of a fragmented message
        ws_message_len = 0;
      }

      if (info->index + len <= ws_max_len - 1) {
        // Append the current fragment to the message buffer
        memcpy(ws_message + info->index, data, len);
        ws_message_len = info->index + len;

        if (info->final) {
          // Message is complete
          ws_message[ws_message_len] = '\0';  // Null-terminate the message
          onMessage(client);                  // Process the complete message
          ws_message_len = 0;                 // Reset for the next message
        }
      } else {
        // Message too large or buffer overflow
        Serial.println("Error: Message exceeds buffer size");
        ws_message_len = 0;
      }
    }
  });
}

void PipoSocket::onMessage(AsyncWebSocketClient* client) {
  if (paused)
    return;
  try {
    char command[16];
    int offset = 0;
    int i = 0;
    for (i = 0; i < ws_message_len; i++) {
      if (ws_message[i] == ':') {
        command[offset] = 0;
        break;
      }
      command[offset++] = ws_message[i];
    }
    if (strcmp("config", command) == 0) {
      config.setValue(ws_message + offset + 1, ws_message_len - offset - 1);
      config.apply(engine, osc, true);
    } else if (strcmp("configs", command) == 0) {
      config.setValues(ws_message + offset + 1, ws_message_len - offset - 1);
      config.apply(engine, osc, true);
    } else if (strcmp("save", command) == 0) {
      config.save();
    } else if (strcmp("monitor", command) == 0) {
      input_sensor.monitor_axis(ws_message + offset + 1);
    } else if (strcmp("scanrssi", command) == 0) {
      wifi.triggerRefreshRSSI();
    } else if (strcmp("rssi", command) == 0) {
      toSend[0] = true;
    }
  } catch (const std::exception& e) {
    logs.writeError("error on message" + String(e.what()));
    Serial.println("error on message");
    Serial.println(e.what());
  }
}

void PipoSocket::start(AsyncWebSocket* ws) {
  this->ws = ws;
  this->ws->enable(true);
}
void PipoSocket::sendNoteOn(int note, int velocity, int channel) {
  if (ws == nullptr || paused)
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
  if (ws == nullptr || paused)
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
  if (ws == nullptr || paused)
    return;
  String msg = "sensor";
  msg += axis.c_str();
  msg += ",";
  msg += value;
  ws->textAll(msg.c_str());
}

void PipoSocket::loop() {
  if (ws == nullptr || paused)
    return;
  auto clients = ws->getClients();
  if (clients.length() == 0)
    return;
  bool canSend = true;
  for (AsyncWebSocketClient* c : clients) {
    if (c->status() != WS_CONNECTED)
      continue;
    canSend = canSend && c->canSend();
  }
  // Do not try to send if any client is not ready
  // Because the lib still allocates memory for the message
  if (!canSend)
    return;
  unsigned long now = millis();
  // if (now - lastCleanTime > 500) {
  //   for (AsyncWebSocketClient* c : ws->getClients()) {
  //     if (c->freeSpace() < 30) {
  //       Serial.printf("Client ID = %u, Queue Length = %u\n",
  //                     c->id(), c->freeSpace());
  //     }
  //   }
  //   lastCleanTime = now;
  // }
  std::string message = "fps,";
  message += std::to_string((float)iterations);
  message += ",";
  message += std::to_string((float)now - lastSendTime);
  iterations = 1;
  lastSendTime = now;
  const auto& sensor_dat = input_sensor.get_sensor_dat_map();
  for (auto const& pair : sensor_dat) {
    if (!pair.second.ws_monitor)
      continue;
    string axis_name = pair.first;
    float sensor_val = input_sensor.get_value(axis_name);
    bool sensor_bool = input_sensor.get_bool_value(axis_name);
    float sensor_min = input_sensor.get_limit_min(axis_name);
    float sensor_max = input_sensor.get_limit_max(axis_name);

    // check if axis is enabled, outside deadzone and not disabled
    if (!input_sensor.test_outside_deadzone(axis_name))
      continue;

    if (toSend[0]) {
      message += "\nrssi";
      message += ",";
      message += std::to_string((int)wifi.getRSSI());
      toSend[0] = false;
    }
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
void PipoSocket::stop() {
  Serial.print("Closing with clients: ");
  Serial.println(ws->count());
  this->ws->cleanupClients();
  this->ws->closeAll();
  this->ws->enable(false);
  this->ws = nullptr;
}
void PipoSocket::pause() {
  // TODO: check if necessary
  this->ws->cleanupClients();
  this->ws->closeAll();

  paused = true;
}
void PipoSocket::resume() {
  paused = false;
}
