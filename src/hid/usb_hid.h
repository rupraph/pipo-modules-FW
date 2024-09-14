#ifndef _USB_HID_H_
#define _USB_HID_H_

#include <Adafruit_TinyUSB.h>
#include "HW_CONFIG.h"

// temporarily test hid here

class usb_hid {
 public:
  usb_hid();

  int hid_mode = 0;  //0=gamepad, 1=mouse, 2=keyboard

  void usb_hid_setup();
  void usb_hid_update(const void* report);
};

uint8_t const gamepad_hid_report[] = {TUD_HID_REPORT_DESC_GAMEPAD()};
uint8_t const mouse_hid_report[] = {TUD_HID_REPORT_DESC_MOUSE()};
uint8_t const key_hid_report[] = {TUD_HID_REPORT_DESC_KEYBOARD()};

#endif