# WiFi battery-saving mode

Without this mode, the Pipo runs its WiFi in AP + STA mode all the time: it hosts its own access point (AP) and also connects to a known WiFi network (STA). Hosting an AP keeps the radio awake permanently, which is the worst case for battery life.

Battery-saving mode turns the AP off when nobody needs it. The radio then either:
- stays connected to the known network only, with modem sleep, or
- turns off completely, in MIDI mode when there is no known network.

The mode is off by default. Enable it in **Settings → WiFi battery saving** (`Wifi_ps_ena`). Changes apply as soon as you save; no reboot is needed.

## The three WiFi states

| State | AP | Known network (STA) | Config page reachable at | Battery |
|---|---|---|---|---|
| **AP + STA** | on | connected if available | the AP, and `pipo-<name>.local` on the known network | worst (radio always awake) |
| **STA only** | off | connected | `pipo-<name>.local` on the known network | good (modem sleep) |
| **Off** | off | off | nowhere until the AP is reopened | best (BLE MIDI still works) |

## When the AP stays on

The AP stays on while **any** of these is true:

1. **Boot window.** For `Wifi_ap_boot_s` (default 60 s) after power-on.
2. **The config page is in use.** The page counts as in use for `Wifi_ap_idle_s` (default 120 s) after the last sign of use. A sign of use is any of:
   - the page being visible on screen (it sends a heartbeat every 10 s while visible, and again when it comes back on screen);
   - loading the page;
   - editing a setting, saving, changing WiFi settings, or starting a calibration.
3. **The AP was just reopened.** For `Wifi_ap_idle_s` after the AP comes back, whether from a button long press or the network-lost fallback.
4. **OSC mode with no known network.** The AP is the only network the OSC data can go out on, so it stays on indefinitely.
5. **OSC mode where the OSC target is a device on the AP.** The OSC target IP (`OSC_IP`) is on the AP's subnet (for example the default `192.168.10.x`), so the AP carries the OSC data. This doesn't apply if the known network uses the same address range, because the target is then assumed to be on the known network.

These things do **not** keep the AP on:
- **A phone or computer that is merely connected to the AP.** A user who configured the Pipo and forgot to disconnect their phone is disconnected once the idle time is over.
- **A config page left open but hidden:** a locked screen, another app in front, or a background browser tab. The heartbeat stops as soon as the page is hidden.
- **Background polling from the page:** battery level, `/ping`, WiFi state and network list, calibration status. The page also pauses its battery polling while hidden.

## When the AP turns off

The AP turns off only when **none** of the conditions above holds, and then:

| Situation | Result |
|---|---|
| Known network connected for at least 5 s | **STA only.** A phone still on the AP is disconnected. The page stays reachable at `pipo-<name>.local` on the known network. |
| MIDI mode, no known network | **Off.** WiFi is fully off and BLE MIDI keeps working if enabled. |
| OSC mode, no known network | Never; see condition 4. |

In practice, after the boot window:
- **OSC user with a known network:** the AP turns off about 2 minutes after they close or lock the config page.
- **MIDI user without a known network:** WiFi turns off about 2 minutes after they close or lock the page.
- **OSC user without a known network:** the AP never turns off.

## How the AP comes back

| Trigger | Result |
|---|---|
| **Long press on play/pause** for `Wifi_ap_btn_ms` (default 2 s) | The AP reopens; from **Off**, the radio turns back on and reconnects to the known network if it's in range. The AP then stays on for `Wifi_ap_idle_s`, and longer if the page is used. |
| **Known network lost** while in STA only, with no reconnection within `Wifi_sta_lost_s` (default 15 s) | The AP reopens automatically, so the Pipo never becomes unreachable. |
| **Reboot** | A fresh boot window. |
| **Turning the setting off** | The AP is restored once. After that the WiFi mode is left alone, so a manual WiFi mode choice is respected. |

A short press on play/pause still toggles pause as before.

The long press is not available in two cases, where only a reboot brings the AP back:
- **Buttons are locked** (`Button_disa`).
- **Boards without the button task:** hardware rev 1.0 (`motion`, `range` and `analog` environments). Rev 1.1 and analog rev 2 have it.

## WiFi LED

| LED pattern | Meaning |
|---|---|
| Slow pulse (2 s) | AP on, connected to the known network |
| Fast pulse (1 s) | AP on, a device connected to the AP, no known network |
| Blink (1 s, 20% on) | AP on, no connection |
| **Short flash every 3 s** | **STA only:** AP off, connected to the known network |
| Off | WiFi off |

A device joining or leaving the AP while the known network is connected doesn't change the LED, because the known network takes priority. The LED changes when the AP actually turns off.

## Settings

| Key | Default | Range | Meaning |
|---|---|---|---|
| `Wifi_ps_ena` | `false` | — | Enables battery-saving mode |
| `Wifi_ap_boot_s` | 60 | 10–600 s | AP time after boot |
| `Wifi_ap_idle_s` | 120 | 30–1800 s | AP time after the last page use or AP reopen |
| `Wifi_sta_lost_s` | 15 | 5–120 s | Grace period before the AP reopens when the known network is lost |
| `Wifi_ap_btn_ms` | 2000 | 500–5000 ms | Play/pause hold time to reopen the AP |

Configs and presets without these keys use the defaults.

## Safeguards against unwanted switching

- **The known network must be stable for 5 s** before the AP is dropped. A connection that flaps doesn't kill the AP.
- **At least 3 s between two switches** prevents oscillation. A button press skips this wait.
- **No switching while the server is paused** for a config save, a WiFi connect or a manual mode change.
- **The STA connection is kept** when the AP is added or removed. Only turning WiFi fully off and back on restarts the radio.

## Debugging

Every switch logs at info level on the serial monitor (`npm run serial`):

```
WiFi power: APSTA -> STA_ONLY
Disabling AP (STA only, modem sleep)
WiFi power: AP requested by button
Turning WiFi radio on (APSTA)
```

WebSocket clients that stop answering pings (a device that left the network) are closed after about 10 s and logged as `WebSocket client not answering pings, closing`. This only frees the connection; it doesn't decide anything about the AP.

To test faster, lower `Wifi_ap_idle_s` to 30 and `Wifi_ap_boot_s` to 10.

## Code map

| What | Where |
|---|---|
| Decision rules (pure function, unit tested) | `src/wifi/wifi_policy.h`, tests in `test/test_wifi_policy/` |
| Inputs, switching, and the AP and radio on/off actions | `src/wifi/pipowifi.cpp`, "Battery-saving mode" section |
| Page-use signals | `src/server/server.cpp` (HTTP middleware), `src/server/pipo_socket.cpp` (WebSocket events, `alive` command) |
| Heartbeat sender | `web-config/src/pipoio.ts` (`sendAlive`) |
| Button | `src/main.cpp` (callback), `src/hw_ui.cpp` (hold time) |
| LED patterns | `src/hw_ui.cpp` (`monitor_wifiBT_flags`) |
