#include "usb_hid.h"

// UNDER CONSTRUCTION

usb_hid hidio;

Adafruit_USBD_HID usb_hid_port;
// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t

usb_hid::usb_hid() {
  // usb_hid_port;
  // hid_gamepad_report_t gp;
}

void usb_hid::setup(int hid_mode) {
  // if willing to have boot functionality, missing something like:
  // usb_hid_port.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);

  // Set HID Report descriptor
  this->hid_mode = hid_mode;
  // usb_hid_port.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD); low level protocol (bios, etc...)
  usb_hid_port.setPollInterval(2);
  switch (hid_mode) {
    case 0:
      usb_hid_port.setReportDescriptor(gamepad_hid_report,
                                       sizeof(gamepad_hid_report));
      usb_hid_port.setStringDescriptor("PiPo Gamepad");
      break;
    case 1:
      usb_hid_port.setReportDescriptor(mouse_hid_report,
                                       sizeof(mouse_hid_report));
      usb_hid_port.setStringDescriptor("PiPo Mouse");
      break;
    case 2:
      usb_hid_port.setReportDescriptor(key_hid_report, sizeof(key_hid_report));
      usb_hid_port.setStringDescriptor("PiPo Keyboard");
      break;
  }

  // usb_hid_port.setReportCallback(NULL, NULL); to have feedback like switching
  // leds on/off

  usb_hid_port.begin();

  // wait until device mounted. then timeout and report not mounted
  unsigned long timeout = millis() + 3000;

  while (!TinyUSBDevice.mounted() && millis() <= timeout) {
    delay(1);
  }
  if (!TinyUSBDevice.mounted()) {
    Serial.println("USB HID not mounted");
  } else {
    Serial.println("USB HID mounted");
  }
  if (DEBUG_HEAP)
    pipoDebugHeap();
}

void usb_hid::update() {
  if (!usb_hid_port.ready())
    return;

  // Remote wakeup
  if (TinyUSBDevice.suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  switch (hid_mode) {
    // case 0:
    //   usb_hid_port.sendReport(0, &gp, sizeof(gp));
    //   break;
    case 1:
      usb_hid_port.mouseReport(0, mouse.buttons, mouse.x, mouse.y, mouse.wheel,
                               mouse.pan);
      break;
    case 2:
      usb_hid_port.keyboardReport(0, 0, kb_keycodes);
      break;
  }
}

void usb_hid::mouse_update(string key, int8_t value, bool bool_value) {
  if (key == "X") {
    mouse.x = value;
  }
  if (key == "Y") {
    mouse.y = value;
  }
  if (key == "WHEEL") {
    mouse.wheel = value;
  }
  if (key == "PAN") {
    mouse.pan = value;
  }
  if (key == "LEFT" && bool_value) {
    mouse.buttons |= MOUSE_BUTTON_LEFT;
  } else if (key == "LEFT" && !bool_value) {
    mouse.buttons &= ~MOUSE_BUTTON_LEFT;
  }
  if (key == "RIGHT" && bool_value) {
    mouse.buttons |= MOUSE_BUTTON_RIGHT;
  } else if (key == "RIGHT" && !bool_value) {
    mouse.buttons &= ~MOUSE_BUTTON_RIGHT;
  }
}

// void usb_hid::set_gamepad_report_value(string key, int value) {
//   // memo
//   //  gp.hat = 0-8 HAT/DPAD
//   //  gp.buttons = Button Bit Mask (bit 0 to 31)

//   auto it = GamepadMap.find(key);
//   if (it != GamepadMap.end()) {
//     gp.*it->second = value;
//   } else {
//     Serial.print("gamepad key not set ");
//     Serial.println(key.c_str());
//   }
// }

// for keyboard, key is keycode. keycode to name is maintained in config client
void usb_hid::keyboard_set_press(string address) {
  //this derives from keyboardPress in Adafruit_USBD_HID.cpp
  string map_address = address;
  if (map_address.length() == 5) {
    char ch = map_address[4];
    uint8_t modifier = 0;
    uint8_t uch = (uint8_t)ch;
    // if (_ascii2keycode[uch][0]) {
    //   modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
    // }
    Serial.println(_ascii2keycode[uch][1]);
    kb_keycodes[keycodes_index] = _ascii2keycode[uch][1];
  } else {  // deal with KEY_UP / KEY_DOWN / KEY_LEFT / KEY_RIGHT
    if (map_address == "KEY_UP") {
      kb_keycodes[keycodes_index] = HID_KEY_ARROW_UP;
    }
    if (map_address == "KEY_DOWN") {
      kb_keycodes[keycodes_index] = HID_KEY_ARROW_DOWN;
    }
    if (map_address == "KEY_LEFT") {
      kb_keycodes[keycodes_index] = HID_KEY_ARROW_LEFT;
    }
    if (map_address == "KEY_RIGHT") {
      kb_keycodes[keycodes_index] = HID_KEY_ARROW_RIGHT;
    }
    if (map_address == "KEY_ENTER") {
      kb_keycodes[keycodes_index] = HID_KEY_ENTER;
    }
    if (map_address == "KEY_ESC") {
      kb_keycodes[keycodes_index] = HID_KEY_ESCAPE;
    }
    if (map_address == "KEY_SPACE") {
      kb_keycodes[keycodes_index] = HID_KEY_SPACE;
    }
  }

  if (keycodes_index < 5) {
    keycodes_index++;
  } else {
    Serial.println("keyboard key count exceeded, only using the first 6 keys");
  }
}

void usb_hid::keyboard_release() {
  // Reset keycodes
  for (int i = 0; i < 6; ++i) {
    kb_keycodes[i] = 0;
  }
  keycodes_index = 0;
}

// void usb_hid::keyboard_update() {
//   //modifier not taken into account for now
//   if (!usb_hid_port.ready())
//     return;

//   usb_hid_port.keyboardReport(0, 0, kb_keycodes);

//   // if (sizeof(kb_keycodes) > 0) {
//   //   key_pressed_previously = true;
//   //   usb_hid_port.keyboardReport(0, 0, kb_keycodes.data());
//   //   kb_keycodes.clear();
//   // } else {
//   //   if (key_pressed_previously) {
//   //     usb_hid_port.keyboardRelease(0);
//   //     key_pressed_previously = false;
//   //   }
//   // }
// }

void usb_hid::set_enabled(bool ena) {
  enabled = ena;
}
