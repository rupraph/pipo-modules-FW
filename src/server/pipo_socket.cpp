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
      taskDelay = 200;  // Weak signal → Send less often
    } else {
      taskDelay = 300;  // Very poor signal → Minimize WebSocket activity
    }

    // Slow down websockets when BLE is connected to avoid conflicts
    if (BTconnected) {
      taskDelay =
          max(taskDelay * 2, 120);  // At least double the delay, minimum 120ms
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

    // Ignore ALL WebSocket activity when paused (e.g., during WiFi changes)
    // The network change will naturally kill the connection
    // The page will reload and reconnect with fresh state
    if (paused) {
      log_v("Ignoring WebSocket event while paused (type=%d)", type);
      return;
    }

    // Ignore all frames from clients marked for termination
    if (closingClients.find(client->id()) != closingClients.end()) {
      log_v("Ignoring frame from terminating client ID=%u", client->id());
      return;
    }

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
      // Remove from our tracking set when disconnect event fires
      closingClients.erase(client->id());
    } else if (type == WS_EVT_ERROR) {
      uint16_t errorCode = *((uint16_t*)arg);
      log_w("WebSocket error: ID=%u code=%u", client->id(), errorCode);
    } else if (type == WS_EVT_PONG) {
    } else if (type == WS_EVT_DATA) {
      if (DEBUG_HEAP)
        pipoDebugHeap("WS: Data");
      AwsFrameInfo* info = (AwsFrameInfo*)arg;

      // Check for invalid opcodes - these indicate corrupted WebSocket stream
      // Valid opcodes: 0=CONTINUATION, 1=TEXT, 2=BINARY, 8=CLOSE, 9=PING, 10=PONG
      if (info->opcode > 10 && info->opcode != WS_DISCONNECT) {
        log_e("WebSocket: Invalid opcode %d - terminating client ID=%u",
              info->opcode, client->id());
        closingClients.insert(client->id());
        client->close(1002, "Protocol error");
        inMsgL = 0;
        return;
      }

      // Only accept TEXT frames for our application protocol
      // Reject BINARY frames to prevent parser corruption
      if (info->opcode == WS_BINARY) {
        log_w("WebSocket: BINARY frame - terminating client ID=%u",
              client->id());
        closingClients.insert(client->id());
        client->close(1003, "Unsupported data");
        inMsgL = 0;
        return;
      }

      // Only process TEXT and CONTINUATION frames
      bool shouldProcess =
          (info->opcode == WS_TEXT || info->opcode == WS_CONTINUATION);

      if (!shouldProcess) {
        // Control frames (PING, PONG) are handled by library, just skip
        log_v("WebSocket: Skipping control/reserved frame (opcode=%d)",
              info->opcode);
        return;
      }

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

          // Validate message is printable text
          bool isPrintable = true;
          for (int i = 0; i < min(inMsgL, 64); i++) {
            if (inMsg[i] < 32 || inMsg[i] > 126) {
              isPrintable = false;
              break;
            }
          }

          if (isPrintable) {
            log_v("WebSocket TEXT message (%d bytes): %.32s%s", inMsgL, inMsg,
                  inMsgL > 32 ? "..." : "");
            onMessage(client);  // Process the complete message
          } else {
            log_w(
                "WebSocket: Non-printable TEXT data (%d bytes) - terminating "
                "ID=%u",
                inMsgL, client->id());
            closingClients.insert(client->id());
            client->close(1003, "Invalid text data");
            inMsgL = 0;
            return;
          }

          inMsgL = 0;  // Reset for the next message
        }
      } else {
        // Message too large or buffer overflow
        log_e("WebSocket: Message exceeds buffer - terminating ID=%u",
              client->id());
        closingClients.insert(client->id());
        client->close(1009, "Message too large");
        inMsgL = 0;
        return;
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

  // Clean up tracking set - remove IDs that are no longer in the clients list
  std::set<uint32_t> currentClientIds;
  for (AsyncWebSocketClient* c : clients) {
    currentClientIds.insert(c->id());
  }

  // Remove IDs from closingClients that are no longer in the actual client list
  // (meaning they've been successfully removed by the library)
  std::set<uint32_t> toRemove;
  for (uint32_t id : closingClients) {
    if (currentClientIds.find(id) == currentClientIds.end()) {
      toRemove.insert(id);
    }
  }
  for (uint32_t id : toRemove) {
    closingClients.erase(id);
  }

  // Now close dead clients, but only if we haven't already asked them to close
  for (AsyncWebSocketClient* c : clients) {
    uint32_t clientId = c->id();

    // Skip if we've already asked this client to close
    if (closingClients.find(clientId) != closingClients.end()) {
      continue;
    }

    // Check if client is dead
    if (c->status() != WS_CONNECTED || c->client() == nullptr) {
      log_i("Removing dead WebSocket client: ID=%u status=%u", clientId,
            c->status());
      c->close();
      closingClients.insert(clientId);  // Track that we've closed this client
    }
  }
}

void PipoSocket::clearAllClients() {
  if (ws == nullptr)
    return;

  // Don't try to force-close clients - this causes iterator invalidation
  // Just clean up dead ones and let paused state prevent new activity
  log_i("Cleaning up dead WebSocket clients (%u total)", ws->count());
  cleanupDeadClients();
  closingClients.clear();
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
    bool foundDelimiter = false;

    for (i = 0; i < inMsgL && offset < 15; i++) {  // Prevent buffer overflow
      if (inMsg[i] == ':') {
        foundDelimiter = true;
        break;
      }
      command[offset++] = inMsg[i];
    }
    command[offset] = 0;  // Ensure null termination

    // Ignore malformed messages without delimiter or with oversized commands
    if (!foundDelimiter && offset >= 15) {
      log_w("WebSocket: Ignoring message with oversized command (>15 chars)");
      return;
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

  unsigned long now = millis();

  // Cleanup dead clients periodically (not every loop iteration)
  if (now - lastCleanTime > CLEANUP_INTERVAL) {
    cleanupDeadClients();
    lastCleanTime = now;
  }

  auto clients = ws->getClients();
  if (clients.length() == 0)
    return;

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
    snprintf(outMsg + strlen(outMsg), remaining, "\nrssi,%d",
             (int)wifi.getRSSI());
  }

  // Append logs if space allows
  if (logs.hasNews() && strlen(outMsg) + logs.length(true) + 7 < outMaxLen) {
    remaining = outMaxLen - strlen(outMsg) - 1;
    snprintf(outMsg + strlen(outMsg), remaining, "\nlogs,%s",
             logs.readLogs(true));
  }

  // Validate message is safe to send (prevent 1002 protocol errors)
  size_t msgLen = strlen(outMsg);
  if (msgLen == 0) {
    return;  // Nothing to send
  }

  // Ensure null termination and prevent buffer overflow
  if (msgLen >= outMaxLen) {
    log_e("Message too large (%zu bytes), truncating", msgLen);
    outMsg[outMaxLen - 1] = '\0';
    msgLen = outMaxLen - 1;
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

// Extra safety: when BLE active, skip if TCP layer also struggling
// This prevents radio conflicts from causing protocol errors
#ifdef INCLUDE_BLE
    if (BTconnected && c->client() && c->client()->space() < 512) {
      continue;  // Give BLE priority, will retry next iteration
    }
#endif

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
  // Clear all clients to prevent rapid reconnection storms when resuming
  // This ensures a clean slate when the page becomes visible again
  clearAllClients();
}
