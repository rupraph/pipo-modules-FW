#ifndef _USB_HID_H_
#define _USB_HID_H_

#include <Adafruit_TinyUSB.h>

// temporarily test hid here

int hid_mode = 0; //0=gamepad, 1=mouse, 2=keyboard

uint8_t const gamepad_hid_report[] = {
  TUD_HID_REPORT_DESC_GAMEPAD()
};
uint8_t const mouse_hid_report[] = {
  TUD_HID_REPORT_DESC_MOUSE()
};
uint8_t const key_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid;
// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t
hid_gamepad_report_t    gp;

#endif