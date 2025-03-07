#include "osc/osc_handler.h"

OSC_handler osc;

/// @brief setup the OSC handler
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
    pipoDebugHeap();
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
  if (isStarted) {
    return;
  }

  if (dest_ip == IPAddress(0, 0, 0, 0) || out_port == 0) {
    Serial.println("Can't start OSC, No destination IP or port defined");
    return;
  } else {
    Serial.println("Starting OSC");
    Serial.print("OSC sending to IP: ");
    Serial.println(dest_ip.toString());
    Serial.print("on port:");
    Serial.println(String(out_port));
    Udp.begin(out_port);
    isStarted = true;
    Serial.println("OSC started");
  }
}

void OSC_handler::stop() {
  if (!isStarted) {
    return;
  }
  Udp.stop();
  isStarted = false;
}

// assume data format is /pwm/1. With duty cycle btw 0 and 1
void send_to_analog(OSCMessage& msg, int addrOffset) {
  String address = msg.getAddress();
  String deststring = address.substring(addrOffset + 1);
  // int dest = deststring.toInt();
  if (LOG_RECEIVED_OSC) {
    Serial.print("address: ");
    Serial.print(address);
    Serial.print(" offset");
    Serial.println(addrOffset);
    Serial.print("dest: ");
    Serial.println(deststring);
    Serial.print("value:  ");
    if (msg.isFloat(0)) {
      Serial.println(msg.getFloat(0));
    } else if (msg.isInt(0)) {
      Serial.println(msg.getInt(0));
    } else {
      Serial.println("not a float or int");
    }
  }
#ifdef PIPO_ANALOG
  analog_out.set_value(deststring.c_str(), msg.getFloat(0));
#endif
}

void OSC_handler::receive() {
  if (isStarted == false || !enabled) {
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
  if (!isStarted || !enabled) {
    return;
  }
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
  this->enabled = ena;
}

bool OSC_handler::is_enabled() {
  return enabled;
}

bool OSC_handler::is_started() {
  return isStarted;
}
