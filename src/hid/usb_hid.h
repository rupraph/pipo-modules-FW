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
  int hid_mode = 0;  //0=gamepad, 1=mouse, 2=keyboard

  void usb_hid_setup();
  void usb_hid_update();

  void set_mouse_report_value(string key, int value);
  void set_gamepad_report_value(string key, int value);

  void keyboard_update();
  void keyboard_set_press(string keycode);
  void keyboard_release();

  void set_hid_mode(int mode);
  void set_enabled(bool ena);
  bool get_enabled();
  int get_hid_mode();

 private:
  hid_gamepad_report_t gp;

  hid_mouse_report_t mouse;

  using MousePtr = int8_t hid_mouse_report_t::*;
  std::unordered_map<std::string, MousePtr> MouseMap = {
      //{"buttons", &hid_mouse_report_t::buttons},
      {"x", &hid_mouse_report_t::x},
      {"y", &hid_mouse_report_t::y},
      {"wheel", &hid_mouse_report_t::wheel},
      {"pan", &hid_mouse_report_t::pan}};

  using GamepadPtr = int8_t hid_gamepad_report_t::*;
  std::unordered_map<std::string, GamepadPtr> GamepadMap = {
      //{"buttons", &hid_gamepad_report_t::buttons},
      {"x", &hid_gamepad_report_t::x},   {"y", &hid_gamepad_report_t::y},
      {"z", &hid_gamepad_report_t::z},   {"rz", &hid_gamepad_report_t::rz},
      {"rx", &hid_gamepad_report_t::rx}, {"ry", &hid_gamepad_report_t::ry},
      //{"hat", &hid_gamepad_report_t::hat}
  };

  //keyboard
  hid_keyboard_report_t kb;
  //   std::vector<uint8_t> kb_keycodes;
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