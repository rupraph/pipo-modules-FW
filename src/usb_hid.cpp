#include "usb_hid.h"


// NOT TESTED SINCE MOVED in these seprate files. mouse and gamepad were working working
void usb_hid_setup()
{
    usb_hid.setPollInterval(2);
    // Set HID Report descriptor
    switch (hid_mode)
    {
    case 0:
        usb_hid.setReportDescriptor(gamepad_hid_report, sizeof(gamepad_hid_report));
        break;
    case 1:
        usb_hid.setReportDescriptor(mouse_hid_report, sizeof(mouse_hid_report));
        break;
    case 2:
        usb_hid.setReportDescriptor(key_hid_report, sizeof(key_hid_report));
        break;
    }
    usb_hid.setStringDescriptor("TinyUSB HID Keyboard");
    usb_hid.begin();
    // wait until device mounted
    while (!TinyUSBDevice.mounted())
        delay(1);
}


void hid_update()
{
    if ( !usb_hid.ready() ) return;
        // Joystick 1 DOWN
        Serial.println("Joystick 1 DOWN");
        gp.x = 0;
        gp.y = 127;
        usb_hid.sendReport(0, &gp, sizeof(gp));
        delay(2000);
}