#include "osc/osc_handler.h"

OSC_handler osc;

/// @brief setup the OSC handler
void OSC_handler::setup() {

  set_config();
  if (config.general_config["OSC_ENA"]) {
    Serial.print("OSC sending to IP: ");
    Serial.println(dest_ip.toString());
    Serial.print("on port:");
    Serial.println(String(out_port));
  }
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
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

/// @brief start the UDP connection.
void OSC_handler::start() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Can't start OSC, WiFi is not connected"));
    return;
  }

  if (dest_ip == IPAddress(0, 0, 0, 0) || out_port == 0) {
    Serial.println(F("Can't start OSC, No destination IP or port set"));
    return;
  } else {
    Serial.println(F("Starting OSC"));
    Udp.begin(out_port);
    isStarted = true;
    Serial.println(F("OSC started"));
  }
}

void OSC_handler::stop() {
  if (!isStarted) {
    return;
  }
  Udp.stop();
  isStarted = false;
}

// assume data format is /pwm/1 100

void OSC_handler::receive() {
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      const char* address = msg.getAddress();
      if (strstr(address, "/pwm") != nullptr) {
        Serial.println("PWM message received");
        // hw_output.set_pwm(address[-1], msg.getInt(0));
        Serial.println("PWM message received" + msg.getInt(0));
      }
    } else {
      // error = msg.getError();
      Serial.print("error: ");
      // Serial.println(error);
    }
  }
}

/// @brief use to update the destination IP
void OSC_handler::set_dest_ip(string ip) {
  IPAddress new_ip;
  new_ip.fromString(ip.c_str());
  if (!isStarted) {
    dest_ip = new_ip;
  }
  if (isStarted) {
    stop();
    dest_ip = new_ip;
    start();
  }
}

/// @brief use to update the output port
void OSC_handler::set_out_port(int port) {
  if (!isStarted) {
    out_port = port;
  }
  if (isStarted) {
    stop();
    out_port = port;
    start();
  }
}

void OSC_handler::send_osc_message(string address, float value) {
  if (dest_ip != IPAddress(0, 0, 0, 0) && out_port != 0) {
    //OSCMessage msg(("/" + string(PIPO_TYPE) + "/" + address).c_str()); default address

    if (address[0] != '/') {
      address = "/" + address;
    }
    OSCMessage msg((address).c_str());
    msg.add(value);
    Udp.beginPacket(dest_ip, out_port);
    msg.send(Udp);
    Udp.endPacket();
    hwui.init_blink_once(SEND_LED, NOTE_BLINK_TIME, NOTE_BLINK_BRIGHTNESS);
    msg.empty();
  } else {
    Serial.println(F("No destination IP or port set"));
  }
}

void OSC_handler::set_enabled(bool ena) {
  if (ena) {
    enabled = true;
    if (!isStarted) {
      start();
    }

  } else {
    enabled = false;
    stop();
  }
}

bool OSC_handler::get_enabled() {
  return enabled;
}
