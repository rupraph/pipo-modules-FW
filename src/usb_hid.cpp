#include "usb_hid.h"


Adafruit_USBD_HID usb_hid;
// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t


int hid_mode=0;

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
    usb_hid.setStringDescriptor("PiPo HID");
    usb_hid.begin();
    // wait until device mounted
    while (!TinyUSBDevice.mounted())
        delay(1);
}
 /// Find way on assigning sensor axis to game or mouse axis. this mapping should likely be in engine or a seperate calss than here.

void usb_hid_update(const void *report)
{   
    if ( !usb_hid.ready() ) return;

    switch (hid_mode)
    {
    case 0:
        hid_gamepad_report_t* gp = (hid_gamepad_report_t*)report;

        // Print the value of gp->x
        Serial.print("gp.x: ");
        Serial.println(gp->x);
        usb_hid.sendReport(0, gp, sizeof(*gp));
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
    //delay(2000);
    
}