#include <server/pipo_socket.h>

// takes 2-3 ms for motion
void websocketTask(void* pvParameters) {
  esp_task_wdt_add(NULL);
  for (;;) {
    if (!pipoNetworkReady()) {
      vTaskDelay(pdMS_TO_TICKS(500));
      continue;
    }
    int rssi = wifi.getRSSI();
    int taskDelay;
    // Adjust task delay based on RSSI
    if (rssi > -65) {
      taskDelay = 40;  // Strong signal → High frequency
    } else if (rssi > -70) {
      taskDelay = 80;  // Medium signal → Reduce frequency
    } else if (rssi > -80) {
      taskDelay = 250;  // Weak signal → Send less often
    } else {
      taskDelay = 500;  // Very poor signal → Minimize WebSocket activity
    }

    pipoSocket.loop();
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(taskDelay));
  }
}

PipoSocket pipoSocket;
PipoSocket::PipoSocket() {
  this->ws = nullptr;
}
void PipoSocket::setup() {
  this->ws->onEvent([&](AsyncWebSocket* server, AsyncWebSocketClient* client,
                        AwsEventType type, void* arg, uint8_t* data,
                        size_t len) {
    log_v("WebSocket running on core: %d", xPortGetCoreID());
    if (type == WS_EVT_CONNECT) {
      // Rate limiting: reject connections that are too rapid
      if (!shouldAcceptConnection(client)) {
        log_w("Rejecting rapid reconnection from IP=%s (cooldown active)",
              client->remoteIP().toString().c_str());
        client->close(1008, "Too many connections");  // Policy violation
        return;
      }

      log_i("WebSocket client connected: ID=%u IP=%s", client->id(),
            client->remoteIP().toString().c_str());
      enforceOneClient(client);

      // Update rate limiting state
      lastConnectionTime = millis();
      lastClientIP = client->remoteIP();
    } else if (type == WS_EVT_DISCONNECT) {
      log_i("WebSocket client disconnected: ID=%u", client->id());
    } else if (type == WS_EVT_ERROR) {
      uint16_t errorCode = *((uint16_t*)arg);
      log_w("WebSocket error: ID=%u code=%u", client->id(), errorCode);
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
        log_e("WebSocket: Message exceeds buffer size");
        inMsgL = 0;
      }
      if (DEBUG_HEAP)
        pipoDebugHeap("WS : End Data");
    }
  });
}

void PipoSocket::cleanupDeadClients() {
  if (ws == nullptr)
    return;

  auto clients = ws->getClients();
  for (AsyncWebSocketClient* c : clients) {
    // Remove clients that are disconnected/disconnecting or have null TCP client
    if (c->status() != WS_CONNECTED || c->client() == nullptr) {
      log_i("Removing dead WebSocket client: ID=%u status=%u", c->id(),
            c->status());
      c->close();
    }
  }
}

bool PipoSocket::shouldAcceptConnection(AsyncWebSocketClient* newClient) {
  if (ws == nullptr)
    return false;

  unsigned long now = millis();

  // If this is the first connection, always accept
  if (lastConnectionTime == 0) {
    return true;
  }

  // During WiFi transitions (e.g., AP to STA), be more lenient
  // Allow connections after the initial transition period
  unsigned long minInterval = MIN_CONNECTION_INTERVAL;
  if (wifi.isChangingAP) {
    minInterval = 200;  // More lenient during AP changes
  }

  // Check if enough time has passed since last connection
  unsigned long timeSinceLastConnection = now - lastConnectionTime;
  if (timeSinceLastConnection < minInterval) {
    // Same IP trying to reconnect too quickly
    if (newClient->remoteIP() == lastClientIP) {
      return false;
    }
  }

  return true;
}

void PipoSocket::enforceOneClient(AsyncWebSocketClient* newClient) {
  if (ws == nullptr)
    return;

  auto clients = ws->getClients();
  for (AsyncWebSocketClient* c : clients) {
    // Close all existing clients except the new one
    if (c->id() != newClient->id()) {
      log_i(
          "Closing old WebSocket client ID=%u to enforce single-client policy",
          c->id());
      c->close();
    }
  }
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
    log_e("WebSocket message error: %s", e.what());
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

  // Handle low memory case
  if (ESP.getFreeHeap() < 30000) {
    log_w("⚠️ Low Memory: Skipping WebSocket Messages");
    return;
  }

  // Proactively cleanup dead clients every iteration
  cleanupDeadClients();

  auto clients = ws->getClients();
  if (clients.length() == 0)
    return;

  unsigned long now = millis();

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

    // if (!input_sensor.test_outside_deadzone(axis_name))
    //   continue;

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

  // Send to connected clients with defensive checks
  for (AsyncWebSocketClient* c : clients) {
    // Verify client is in a valid state before sending
    if (c == nullptr || c->client() == nullptr) {
      continue;
    }

    if (c->status() != WS_CONNECTED) {
      continue;
    }

    if (!c->canSend()) {
      continue;  // Queue is full, skip silently (will retry next iteration)
    }

    c->text(outMsg);
  }
}

void PipoSocket::stop() {
  if (ws == nullptr)
    return;

  log_i("Stopping WebSocket with %u clients", ws->count());

  // First disable to prevent new connections
  ws->enable(false);

  // Close all clients gracefully
  ws->closeAll();

  // Give time for close frames to be sent
  delay(100);

  // Final cleanup
  cleanupDeadClients();

  this->ws = nullptr;
}
void PipoSocket::pause() {
  paused = true;
}
void PipoSocket::resume() {
  paused = false;
}
