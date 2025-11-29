# OTA (Over-The-Air) Firmware Updates

## Overview

OTA updates allow you to upload new firmware to your Pipo device wirelessly, without needing to open the case or press any boot buttons. This is the recommended method for firmware updates on production devices.

## How It Works

1. Your Pipo device runs a web server accessible via WiFi
2. Navigate to the OTA update page at `http://pipo-<name>.local/update.html`
3. Upload a `.bin` firmware file through your browser
4. The device automatically installs the firmware and reboots

## Requirements

- Device must be connected to WiFi (STA or AP mode)
- You need the `.bin` firmware file (compiled with PlatformIO)
- A web browser on the same network

## Building Firmware for OTA

Build your firmware with PlatformIO:

```bash
# For analog variant
pio run -e analog_rev2

# The .bin file will be in:
# .pio/build/analog_rev2/firmware.bin
```

## Uploading Firmware

### Method 1: Web Interface (Easiest)

1. Connect to your Pipo device's WiFi network, or ensure it's on your local network
2. Open your browser and navigate to:

   - `http://pipo-<name>.local/update.html` (replace `<name>` with your device name)
   - Or use the IP address: `http://192.168.x.x/update.html`

3. The page shows:

   - Current firmware version
   - Chip model and free space
   - Drag-and-drop upload area

4. Select or drag your `.bin` file
5. Click "Upload Firmware"
6. Wait for the progress bar to complete (usually 10-30 seconds)
7. Device will automatically reboot with new firmware

### Method 2: Command Line (curl)

```bash
# Replace IP address with your device
curl -F "firmware=@.pio/build/analog_rev2/firmware.bin" http://192.168.1.100/ota/upload
```

### Method 3: Programmatic (Python example)

```python
import requests

url = "http://pipo-mydevice.local/ota/upload"
with open(".pio/build/analog_rev2/firmware.bin", "rb") as f:
    files = {"firmware": f}
    response = requests.post(url, files=files)
    print(response.json())
```

## Endpoints

The OTA system provides these HTTP endpoints:

### GET /ota/info

Returns device information:

```json
{
  "currentVersion": "1.2.3",
  "chipModel": "ESP32-S3",
  "flashSize": 8388608,
  "freeSketchSpace": 3145728,
  "sketchSize": 1234567
}
```

### GET /ota/status

Returns current upload status (useful for polling during upload):

```json
{
  "inProgress": true,
  "progress": 524288,
  "size": 1048576
}
```

### POST /ota/upload

Accepts multipart/form-data with firmware binary.
Returns:

```json
{
  "success": true,
  "message": "Update successful, rebooting..."
}
```

## Safety Features

- **Size validation**: Checks if firmware fits in available flash space
- **Write verification**: Each chunk is verified during upload
- **Rollback protection**: If upload fails, device keeps running old firmware
- **Progress tracking**: Real-time upload progress via web UI

## Troubleshooting

### Upload fails immediately

- Check file is a valid `.bin` file
- Ensure device has enough free space (see `/ota/info`)
- Verify you're uploading the correct variant (analog/motion/range)

### Upload stalls at X%

- WiFi connection may be unstable
- Try moving closer to device or AP
- Restart device and try again

### Device doesn't reboot after upload

- Check Serial Monitor for error messages
- Device may be stuck in bootloop if firmware is incompatible
- Re-flash via USB to recover

### Can't access update page

- Verify device WiFi is active (check LEDs)
- Try IP address instead of mDNS (.local)
- Check if device is in correct WiFi mode

## Recovery

If OTA update fails and device won't boot:

1. Open the case
2. Connect via USB
3. Press and hold BOOT button
4. Press RESET button (or power on)
5. Flash firmware via USB: `pio run -e analog_rev2 -t upload`

## Partition Scheme

The device uses a custom partition scheme (`partitions_custom.csv`) with:

- Factory partition for original firmware
- OTA_0 partition for first update
- OTA_1 partition for second update (rollback)

ESP32 automatically switches between OTA_0 and OTA_1, keeping one as backup.

## Version Management

Define firmware version in `platformio.ini`:

```ini
build_flags =
    -DPIPO_FW_VERSION=1.2.3
```

This version appears in:

- `/ota/info` endpoint
- Update web page
- Serial Monitor on boot

## Security Considerations

**Important**: This implementation does NOT include:

- Authentication (anyone on network can upload)
- Firmware signature verification
- HTTPS/TLS encryption

For production deployments, consider adding:

- HTTP Basic Auth or API keys
- MD5/SHA256 firmware verification
- TLS with self-signed certificates
- Allowlist of client MAC addresses

## Implementation Files

- `src/server/ota_update.h` - OTA class declaration
- `src/server/ota_update.cpp` - OTA implementation using ESP32 Update library
- `data/webpage/update.html` - Web UI for uploading firmware
- `src/server/server.cpp` - Integration with AsyncWebServer

## Example Workflow

```bash
# 1. Make code changes
vim src/main.cpp

# 2. Build firmware
pio run -e analog_rev2

# 3. Open browser to device
open http://pipo-mydevice.local/update.html

# 4. Upload .pio/build/analog_rev2/firmware.bin

# 5. Wait for device to reboot (10 seconds)

# 6. Verify new firmware is running
curl http://pipo-mydevice.local/ota/info
```

## Why OTA Instead of USB Bootloader?

The ESP32-S3 with TinyUSB has a hardware limitation:

- **GPIO0 must be LOW during power-on/reset** to enter ROM bootloader
- No software command can override this requirement
- USB bootloader would still require pressing buttons

OTA advantages:

- ✅ No case opening required
- ✅ No button pressing required
- ✅ Works remotely over WiFi
- ✅ Faster than USB (no driver issues)
- ✅ Can update multiple devices simultaneously
- ✅ Automatic rollback if update fails

## Notes

- First-time setup still requires USB flashing
- OTA uses ~50KB RAM during upload
- Upload speed depends on WiFi quality (typically 50-100 KB/s)
- Device becomes unresponsive during upload (LEDs may freeze)
- MIDI/OSC functionality pauses during upload
