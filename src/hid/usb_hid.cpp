#include "usb_hid.h"

// UNDER CONSTRUCTION

Adafruit_USBD_HID usb_hid_port;
// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t

usb_hid::usb_hid() {
  //usb_hid_port;
  //hid_gamepad_report_t gp;
}

void usb_hid::usb_hid_setup() {
  usb_hid_port.setPollInterval(2);
  // Set HID Report descriptor
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

//Todo: Find way on assigning sensor axis to game or mouse axis. this mapping should likely be in engine or a seperate calss than here
void usb_hid::usb_hid_update(const void* report) {
  if (!usb_hid_port.ready())
    return;

  switch (hid_mode) {
    case 0:
      hid_gamepad_report_t* gp = (hid_gamepad_report_t*)report;

      // Print the value of gp->x
      // Serial.print("gp.x: ");
      // Serial.println(gp->x);
      usb_hid_port.sendReport(0, gp, sizeof(*gp));
      break;
      // case 1:
      //     mouse.x = 10;
      //     usb_hid.sendReport(0, &mouse, sizeof(mouse));
      //     break;
      // case 2:
      //     kb.keycode[0] = HID_KEY_A;
      //     usb_hid.sendReport(0, &kb, sizeof(kb));
      //     break;
  }
}