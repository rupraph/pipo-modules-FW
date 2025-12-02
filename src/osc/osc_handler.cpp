#include "osc/osc_handler.h"
#include "shared_flags.h"

OSC_handler osc;

#ifdef PIPO_ANALOG
void oscreceiveTask(void* pvParameters) {
  esp_task_wdt_add(NULL);
  for (;;) {
    if ((staConnected || apConnected) && osc.is_enabled()) {
      osc.receive();
    }
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(10));  // 10ms polling - reasonable for OSC receive
  }
}
#endif

/// @brief Initialize OSC handler - create mutex (call early, before WiFi)
void OSC_handler::init() {
  // Create mutex for thread safety
  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL) {
    log_e("Failed to create OSC mutex!");
  } else {
    log_d("OSC mutex created");
  }
}

/// @brief Setup the OSC handler - configure settings (call after config loaded)
void OSC_handler::setup() {
  set_config();
  // if (config.general_config["OSC_ENA"]) {
  //   // osc.start();
  //   // Serial.print("OSC sending to IP: ");
  //   // Serial.println(dest_ip.toString());
  //   // Serial.print("on port:");
  //   // Serial.println(String(out_port));
  // }
  if (DEBUG_HEAP)
    pipoDebugHeap("OSC: End setup");
}

// Todo pass arguments there
void OSC_handler::set_config() {

  if (config.general_config["OSC_IP"].is<JsonVariant>()) {
    string ip = config.general_config["OSC_IP"];
    set_dest_ip(ip);
    // Serial.println("OSC IP set to: " + dest_ip.toString());
  }
  if (config.general_config["OSC_PORT"].is<JsonVariant>()) {
    set_out_port(config.general_config["OSC_PORT"]);
    // Serial.println("OSC port set to: " + String(out_port));
  }
  if (config.general_config["OSC_ENA"].is<JsonVariant>()) {
    set_enabled(config.general_config["OSC_ENA"]);
    // Serial.println("OSC enabled: " + String(enabled));
  }
}

/// @brief start the UDP connection (internal - must be called with mutex held)
void OSC_handler::ensure_started() {
  if (!isStarted && enabled) {
    Udp.begin(localPort);
    isStarted = true;
    lastConnectionTime = millis();  // Record when we started
    log_i("OSC UDP started, listening on port %d", localPort);
    if (dest_ip != IPAddress(0, 0, 0, 0) && out_port != 0) {
      log_i("OSC sending to IP: %s port: %d", dest_ip.toString().c_str(), out_port);
    }
  }
}

void OSC_handler::stop() {
  if (mutex == NULL) {
    return;  // Not initialized yet, nothing to stop
  }
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    if (isStarted) {
      Udp.stop();
      isStarted = false;
      lastConnectionTime = 0;  // Reset connection time
      log_i("OSC UDP stopped");
    }
    xSemaphoreGive(mutex);
  }
}

// assume data format is /pwm/1. With duty cycle btw 0 and 1
void send_to_analog(OSCMessage& msg, int addrOffset) {
  String address = msg.getAddress();
  String deststring = address.substring(addrOffset + 1);
  // int dest = deststring.toInt();
  if (LOG_RECEIVED_OSC) {
    if (msg.isFloat(0)) {
      log_d("OSC: %s offset %d dest: %s value: %f",
            address.c_str(), addrOffset, deststring.c_str(), msg.getFloat(0));
    } else if (msg.isInt(0)) {
      log_d("OSC: %s offset %d dest: %s value: %d",
            address.c_str(), addrOffset, deststring.c_str(), msg.getInt(0));
    } else {
      log_d("OSC: %s - not a float or int", address.c_str());
    }
  }
#ifdef PIPO_ANALOG
  analog_out.set_value(deststring.c_str(), msg.getFloat(0));
#endif
}

void OSC_handler::receive() {
  if (mutex == NULL) {
    return;  // Not initialized yet
  }
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    ensure_started();

    if (!isStarted || !enabled) {
      xSemaphoreGive(mutex);
      return;
    }

    // Check connection status at receive time
    if (!staConnected && !apConnected) {
      xSemaphoreGive(mutex);
      return;
    }

    // do not try to receive raw udp data in a buffer then transfer to either Bundle or message processing. very tricky and spent long time having constant crashes.
    // keep using as much as possible the library to receive the OSC data.
    OSCBundle bundleIN;
    int size;

    if ((size = Udp.parsePacket()) > 0) {
      // Serial.print("Packet size: ");
      // Serial.println(size);
      while (size--)
        bundleIN.fill(Udp.read());

      if (!bundleIN.hasError()) {
        // Serial.println("OSC route");
        // this will require translators I think
        // bundleIN.route("/pwm", pwm);
        bundleIN.route("/out", send_to_analog);
        // bundleIN.route("/digi", digi);

        // bundleIN.dispatch("/servo", pwm);
      } else {
        OSCErrorCode error = bundleIN.getError();
        Serial.print("Error: ");
        Serial.println(error);
      }
    }

    xSemaphoreGive(mutex);
  }
}

/// @brief use to update the destination IP
void OSC_handler::set_dest_ip(string ip) {
  IPAddress new_ip;
  new_ip.fromString(ip.c_str());
  dest_ip = new_ip;
}

/// @brief use to update the output port
void OSC_handler::set_out_port(int port) {
  out_port = port;
}

// void OSC_handler::send_osc_message(string address, float value) {
//   if (!isStarted || !enabled) {
//     return;
//   }
//   if (dest_ip != IPAddress(0, 0, 0, 0) && out_port != 0) {
//     //OSCMessage msg(("/" + string(PIPO_TYPE) + "/" + address).c_str()); default address
//     address = config.general_config["PipoName"].as<string>() + "/" + address;
//     if (address[0] != '/') {
//       address = "/" + address;
//     }
//     OSCMessage msg((address).c_str());
//     msg.add(value);
//     Udp.beginPacket(dest_ip, out_port);
//     msg.send(Udp);
//     Udp.endPacket();
//     hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
//     msg.empty();
//   } else {
//     Serial.println(F("No destination IP or port set"));
//   }
// }

void OSC_handler::add_to_bundle(string address, float value) {
  if (mutex == NULL) {
    return;  // Not initialized yet
  }
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    ensure_started();

    if (!isStarted || !enabled) {
      xSemaphoreGive(mutex);
      return;
    }

    // Build address: /PipoName/address
    String fullAddress = "/";
    fullAddress += config.general_config["PipoName"].as<const char*>();
    if (address[0] != '/') {
      fullAddress += "/";
    }
    fullAddress += address.c_str();

    bundle.add(fullAddress.c_str()).add(value);

    xSemaphoreGive(mutex);
  }
}

void OSC_handler::send_battery_level(int percentage, bool is_plugged,
                                     bool is_low_battery) {
  if (mutex == NULL) {
    return;  // Not initialized yet
  }
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    ensure_started();

    if (!enabled) {
      xSemaphoreGive(mutex);
      return;
    }

    // Check connection status
    if (!staConnected && !apConnected) {
      xSemaphoreGive(mutex);
      return;
    }

    // Wait for network stack to stabilize after connection
    if (millis() - lastConnectionTime < 100) {
      xSemaphoreGive(mutex);
      return;
    }

    if (dest_ip == IPAddress(0, 0, 0, 0) || out_port == 0) {
      xSemaphoreGive(mutex);
      return;
    }

    String pipoName = config.general_config["PipoName"].as<const char*>();

    // Send battery percentage: /PipoName/Battery <percentage>
    String batteryAddress = "/" + pipoName + "/Battery";
    OSCMessage batteryMsg(batteryAddress.c_str());
    batteryMsg.add((int32_t)percentage);

    // Send plugged state: /PipoName/Plugged <0 or 1>
    String pluggedAddress = "/" + pipoName + "/Plugged";
    OSCMessage pluggedMsg(pluggedAddress.c_str());
    pluggedMsg.add((int32_t)(is_plugged ? 1 : 0));

    // Send low battery state: /PipoName/LowBattery <0 or 1>
    String lowBatteryAddress = "/" + pipoName + "/LowBattery";
    OSCMessage lowBatteryMsg(lowBatteryAddress.c_str());
    lowBatteryMsg.add((int32_t)(is_low_battery ? 1 : 0));

    // Send all three messages
    int packetStatus = Udp.beginPacket(dest_ip, out_port);
    if (packetStatus != 0) {
      batteryMsg.send(Udp);
      Udp.endPacket();
    }
    batteryMsg.empty();

    packetStatus = Udp.beginPacket(dest_ip, out_port);
    if (packetStatus != 0) {
      pluggedMsg.send(Udp);
      Udp.endPacket();
    }
    pluggedMsg.empty();

    packetStatus = Udp.beginPacket(dest_ip, out_port);
    if (packetStatus != 0) {
      lowBatteryMsg.send(Udp);
      Udp.endPacket();
    }
    lowBatteryMsg.empty();

    xSemaphoreGive(mutex);
  }
}

void OSC_handler::send_bundle() {
  if (mutex == NULL) {
    return;  // Not initialized yet
  }
  if (xSemaphoreTake(mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    ensure_started();

    if (!enabled || bundle.size() < 1) {
      bundle.empty();
      xSemaphoreGive(mutex);
      return;
    }

    // Check connection status at send time
    if (!staConnected && !apConnected) {
      // Serial.println(F("OSC: No network connection"));
      bundle.empty();
      xSemaphoreGive(mutex);
      return;
    }

    // Wait for network stack to stabilize after connection (avoid ENOMEM errors)
    if (millis() - lastConnectionTime < 100) {
      // Too soon after connection, network buffers may not be ready
      bundle.empty();
      xSemaphoreGive(mutex);
      return;
    }

    if (dest_ip == IPAddress(0, 0, 0, 0) || out_port == 0) {
      Serial.println(F("No destination IP or port set"));
      bundle.empty();
      xSemaphoreGive(mutex);
      return;
    }

    int packetStatus = Udp.beginPacket(dest_ip, out_port);
    if (packetStatus == 0) {
      Serial.println(F("Failed to start OSC packet"));
      bundle.empty();
      xSemaphoreGive(mutex);
      return;
    }

    if (bundle.hasError()) {
      Serial.print(F("OSC Bundle has error: "));
      Serial.println(bundle.getError());
      bundle.empty();
      xSemaphoreGive(mutex);
      return;
    }

    bundle.send(Udp);
    Udp.endPacket();
    hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
    bundle.empty();  // clear the bundle after sending

    xSemaphoreGive(mutex);
  }
}

void OSC_handler::set_enabled(bool ena) {
  this->enabled = ena;
  if (!ena) {
    stop();  // Stop UDP if OSC is disabled
  }
}

bool OSC_handler::is_enabled() {
  return enabled;
}
bool OSC_handler::is_started() {
  return isStarted;
}
