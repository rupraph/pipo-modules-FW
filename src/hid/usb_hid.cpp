#include "usb_hid.h"

// UNDER CONSTRUCTION

Adafruit_USBD_HID usb_hid_port;
// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t

usb_hid::usb_hid() {
  // usb_hid_port;
  // hid_gamepad_report_t gp;
}

void usb_hid::usb_hid_setup() {
  // if willing to have boot functionality, missing something like:
  // usb_hid_port.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);

  // Set HID Report descriptor
  // hid_mode = config.general_config["HID_MODE"];
  hid_mode = 2;
  // usb_hid_port.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD); low level protocol (bios, etc...)
  usb_hid_port.setPollInterval(2);
  switch (hid_mode) {
    case 0:
      usb_hid_port.setReportDescriptor(gamepad_hid_report,
                                       sizeof(gamepad_hid_report));
      break;
    case 1:
      usb_hid_port.setReportDescriptor(mouse_hid_report,
                                       sizeof(mouse_hid_report));
      break;
    case 2:
      usb_hid_port.setReportDescriptor(key_hid_report, sizeof(key_hid_report));
      break;
  }
  usb_hid_port.setStringDescriptor("PiPo HID");

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
#ifdef DEBUG_HEAP
  Serial.println("Remaining Heap:" + String(ESP.getFreeHeap()));
#endif
}

// Todo: Find way on assigning sensor axis to game or mouse axis. this mapping
// should likely be in engine or a seperate calss than here
void usb_hid::usb_hid_update() {
  if (!usb_hid_port.ready())
    return;

  // Remote wakeup
  if (TinyUSBDevice.suspended()) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  switch (hid_mode) {
    case 0:
      usb_hid_port.sendReport(0, &gp, sizeof(gp));
      break;
    case 1:
      //     mouse.x = 10;
      usb_hid_port.sendReport(0, &mouse, sizeof(mouse));
      break;

      // keyboard should not have a continous report or this will likley cause
      // redundant keypresses
      //  case 2:
      //  //     kb.keycode[0] = HID_KEY_A;
      //      usb_hid_port.sendReport(0, &kb, sizeof(kb));
      //      break;
  }
}

void usb_hid::set_mouse_report_value(string key, int value) {
  // Todo: have to see if "setting" report for mouse is the rigth method
  // since there are function to mousemove and button press
  auto it = MouseMap.find(key);
  if (it != MouseMap.end()) {
    mouse.*it->second = value;
  } else {
    Serial.print("mouse key not set ");
    Serial.println(key.c_str());
  }
}

void usb_hid::set_gamepad_report_value(string key, int value) {
  // memo
  //  gp.hat = 0-8 HAT/DPAD
  //  gp.buttons = Button Bit Mask (bit 0 to 31)

  auto it = GamepadMap.find(key);
  if (it != GamepadMap.end()) {
    gp.*it->second = value;
  } else {
    Serial.print("gamepad key not set ");
    Serial.println(key.c_str());
  }
}

// for keyboard, key is keycode. keycode to name is maintained in config client
void usb_hid::keyboard_set_press(char keycode) {

  usb_hid_port.keyboardPress(0, keycode);

  // if (kb_keycodes.size() >= 6) {
  //   Serial.println("keyboard key count exceeded, only using the first 6 keys");
  // } else {
  //   kb_keycodes.push_back(keycode);
  // }
}

void usb_hid::keyboard_release() {
  if (!usb_hid_port.ready())
    return;
  usb_hid_port.keyboardRelease(0);
}

void usb_hid::keyboard_update() {
  //modifier not taken into account for now
  if (!usb_hid_port.ready())
    return;

  usb_hid_port.keyboardPress(0, 'a');

  // if (sizeof(kb_keycodes) > 0) {
  //   key_pressed_previously = true;
  //   usb_hid_port.keyboardReport(0, 0, kb_keycodes.data());
  //   kb_keycodes.clear();
  // } else {
  //   if (key_pressed_previously) {
  //     usb_hid_port.keyboardRelease(0);
  //     key_pressed_previously = false;
  //   }
  // }
}

void usb_hid::set_hid_mode(int mode) {
  hid_mode = mode;
}

void usb_hid::set_enabled(bool ena) {
  enabled = ena;
}
