#include "osc_handler.h"

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
  if (config.general_config.find("OSC_IP") != config.general_config.end()) {
    string ip = config.general_config["OSC_IP"];
    setDestIp(ip);
    //Serial.println("OSC IP set to: " + dest_ip.toString());
  }
  if (config.general_config.find("OSC_PORT") != config.general_config.end()) {
    setOutPort(config.general_config["OSC_PORT"]);
    //Serial.println("OSC port set to: " + String(out_port));
  }
  if (config.general_config.find("OSC_ENA") != config.general_config.end()) {
    setEnabled(config.general_config["OSC_ENA"]);
    //Serial.println("OSC enabled: " + String(enabled));
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
  Udp.stop();
  isStarted = false;
}

/// @brief use to update the destination IP
void OSC_handler::setDestIp(string ip) {
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
void OSC_handler::setOutPort(int port) {
  if (!isStarted) {
    out_port = port;
  }
  if (isStarted) {
    stop();
    out_port = port;
    start();
  }
}

void OSC_handler::setEnabled(bool ena) {
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

void OSC_handler::sendOscMessage(string address, float value) {
  if (dest_ip != IPAddress(0, 0, 0, 0) && out_port != 0) {
    OSCMessage msg(("/" + string(PIPO_TYPE) + "/" + address).c_str());
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
