#ifndef _USB_HID_H_
#define _USB_HID_H_

#include <Adafruit_TinyUSB.h>
#include "HW_CONFIG.h"
// #include "utils/config.h"
#include <vector>
#include <unordered_map>

//some interesting stuff here: https://arduino-pico.readthedocs.io/en/latest/usb.html

using namespace std;

class usb_hid {
 public:
  usb_hid();

  bool enabled = true;
  int hid_mode;  //0=gamepad, 1=mouse, 2=keyboard

  void setup(int hid_mode);
  void update();

  //   void set_gamepad_report_value(string key, int value);

  //Keyboard
  void keyboard_update();
  void keyboard_set_press(string keycode);
  void keyboard_release();
  //Mouse
  void mouse_update(string key, int8_t value, bool bool_value);

  //Getter//setters
  void set_hid_mode(int mode);
  void set_enabled(bool ena);
  bool get_enabled();
  int get_hid_mode();

 private:
  // Mouse
  hid_mouse_report_t mouse;

  // Gamepad
  hid_gamepad_report_t gp;

  //keyboard
  hid_keyboard_report_t kb;
  int8_t const _ascii2keycode[128][2] = {HID_ASCII_TO_KEYCODE};
  uint8_t kb_keycodes[6];
  int keycodes_index = 0;
  uint8_t kb_keycodes_previously[6];
  bool key_pressed_previously = false;
};

uint8_t const gamepad_hid_report[] = {TUD_HID_REPORT_DESC_GAMEPAD()};
uint8_t const mouse_hid_report[] = {TUD_HID_REPORT_DESC_MOUSE()};
uint8_t const key_hid_report[] = {TUD_HID_REPORT_DESC_KEYBOARD()};

#endif