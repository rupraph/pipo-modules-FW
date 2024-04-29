#ifndef _USB_HID_H_
#define _USB_HID_H_

#include <Adafruit_TinyUSB.h>

// temporarily test hid here

extern int hid_mode; //0=gamepad, 1=mouse, 2=keyboard

uint8_t const gamepad_hid_report[] = {
  TUD_HID_REPORT_DESC_GAMEPAD()
};
uint8_t const mouse_hid_report[] = {
  TUD_HID_REPORT_DESC_MOUSE()
};
uint8_t const key_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

void usb_hid_update(const void *report);
void usb_hid_setup();


#endif