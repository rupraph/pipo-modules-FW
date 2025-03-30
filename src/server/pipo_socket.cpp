#include <server/pipo_socket.h>

PipoSocket pipoSocket;
PipoSocket::PipoSocket() {
  this->ws = nullptr;
}
void PipoSocket::setup(AsyncWebServer* server) {
  // When a thrid client wants to connect, the first one disconnects
  Serial.println("Closing First client (" + String(ws->count()) +
                 ") to allow new connection");

  server->addHandler(ws).addMiddleware(
      [&](AsyncWebServerRequest* request, ArMiddlewareNext next) {
        ws->cleanupClients();
        next();
      });

  this->ws->onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client,
                        AwsEventType type, void* arg, uint8_t* data,
                        size_t len) {
    if (type == WS_EVT_CONNECT) {
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("WS Client disconnected");
      // client->close();
    } else if (type == WS_EVT_ERROR) {
      uint16_t errorCode = *((uint16_t*)arg);
      // client->close(true);
    } else if (type == WS_EVT_PONG) {
    } else if (type == WS_EVT_DATA) {
      if (DEBUG_HEAP)
        pipoDebugHeap("WS: Data");
      AwsFrameInfo* info = (AwsFrameInfo*)arg;

      if (info->index == 0 && !info->final) {
        // Start of a fragmented message
        inMsgL = 0;
      }

      if (info->index + len <= inMaxLen - 1) {
        // Append the current fragment to the message buffer
        memcpy(inMsg + info->index, data, len);
        inMsgL = info->index + len;

        if (info->final) {
          // Message is complete
          inMsg[inMsgL] = '\0';  // Null-terminate the message
          onMessage(client);     // Process the complete message
          inMsgL = 0;            // Reset for the next message
        }
      } else {
        // Message too large or buffer overflow
        Serial.println("Error: Message exceeds buffer size");
        inMsgL = 0;
      }
      if (DEBUG_HEAP)
        pipoDebugHeap("WS : End Data");
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
    for (i = 0; i < inMsgL; i++) {
      if (inMsg[i] == ':') {
        command[offset] = 0;
        break;
      }
      command[offset++] = inMsg[i];
    }
    if (strcmp("config", command) == 0) {
      config.setValue(inMsg + offset + 1, inMsgL - offset - 1);
      config.apply(engine, osc, true);
    } else if (strcmp("configs", command) == 0) {
      config.setValues(inMsg + offset + 1, inMsgL - offset - 1);
      config.apply(engine, osc, true);
    } else if (strcmp("save", command) == 0) {
      config.save();
    } else if (strcmp("monitor", command) == 0) {
      input_sensor.monitor_axis(inMsg + offset + 1);
    } else if (strcmp("scanrssi", command) == 0) {
      wifi.requestRSSI();
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
  if (ws == nullptr || paused || ws->count() == 0)
    return;

  // Handle low memory case
  if (ESP.getFreeHeap() < 30000) {
    Serial.println("⚠️ Low Memory: Skipping WebSocket Messages");
    Serial.println("Clients: " + ws->count());
    return;
  }

  unsigned long now = millis();

  // Send periodic PING to keep connections alive
  if (now - lastPingTime > PING_INTERVAL) {
    // Serial.println("🔄 Sending WebSocket PING");
    // ws->pingAll();
    lastPingTime = now;
  }

  outMsg[0] = 0;
  snprintf(outMsg, outMaxLen, "fps,%.2f,%.2f", (float)iterations,
           (float)(now - lastSendTime));
  iterations = 1;
  lastSendTime = now;

  const auto& sensor_dat = input_sensor.get_sensor_dat_map();
  for (auto const& pair : sensor_dat) {
    if (!pair.second.ws_monitor)
      continue;
    string axis_name = pair.first;
    float sensor_val = input_sensor.get_value(axis_name);
    bool sensor_bool = input_sensor.get_bool_value(axis_name);

    if (!input_sensor.test_outside_deadzone(axis_name))
      continue;

    size_t remaining = outMaxLen - strlen(outMsg) - 1;
    snprintf(outMsg + strlen(outMsg), remaining, "\nsensor,%s,%.2f,%d",
             axis_name.c_str(), sensor_val, sensor_bool);
  }

  // Append RSSI value if space allows
  size_t remaining = outMaxLen - strlen(outMsg) - 1;
  if (remaining > 12) {
    toSend[0] = false;
    snprintf(outMsg + strlen(outMsg), remaining, "\nrssi,%d",
             (int)wifi.getRSSI());
  }

  // Append logs if space allows
  if (logs.hasNews() && strlen(outMsg) + logs.length(true) + 7 < outMaxLen) {
    remaining = outMaxLen - strlen(outMsg) - 1;
    snprintf(outMsg + strlen(outMsg), remaining, "\nlogs,%s",
             logs.readLogs(true));
  }

  ws->textAll(outMsg);
  // auto& clients = ws->getClients();  // référence, pas de copie
  // for (auto& client : clients) {
  //   if (client.status() != WS_CONNECTED)
  //     continue;
  //   if (client.queueLen() > 30)
  //     continue;
  //   client.text(outMsg, strlen(outMsg));
  // }
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
  paused = true;
}
void PipoSocket::resume() {
  paused = false;
}
