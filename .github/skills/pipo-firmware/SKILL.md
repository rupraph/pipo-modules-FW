---
name: pipo-firmware
description: >
  Pipo-single ESP32-S3 firmware project. Sensor-to-MIDI/OSC wireless controllers
  with Svelte web config UI. Covers firmware architecture, RTOS tasks, sensor pipeline,
  engine output, config management, WebSocket protocol, HTTP REST API, WiFi state machine,
  build system, PlatformIO environments, and web frontend. Triggers on: "pipo", "firmware",
  "sensor", "midi", "osc", "config", "websocket", "engine", "wifi", "preset", "calibration",
  "build", "flash", "analog", "motion", "range", "BLE", "LittleFS", "ArduinoJson".
---

# Pipo-Single Firmware — Development Skill

ESP32-S3 firmware + Svelte web UI for wireless sensor-to-MIDI/OSC controllers.
3 device variants (motion/range/analog) share one codebase with compile-time switching.

---

## Project Map

```
src/
  main.cpp              — Boot sequence, RTOS task creation
  engine.cpp/h          — Output multiplexer (MIDI/OSC/HID per-axis)
  HW_CONFIG.h           — Pin defs, variant constants, debug flags
  hw_ui.cpp/h           — LEDs (LEDC/WS2812), buttons, battery monitor
  shared_flags.h        — Volatile inter-task flags
  task-handles.h        — FreeRTOS task handles
  sensors/
    input_sensor.h      — Base Sensor class + SensorDat struct + trigger flags
    acc_sensor.h        — MotionSensor (ICM-20948, 12 axes)
    analog_sensor.h     — AnalogSensor (ADC+touch, 16 axes)
    pipo_range_sensor.h — PipoRangeSensor (ToF, 1 axis)
  midi/
    midi_io.cpp/h       — MIDI output: CC/Note modes, sustain manager
    midiBLE.cpp/h       — BLE MIDI (NimBLE)
    midiUSB.cpp/h       — USB MIDI (TinyUSB)
    midi_translator.h   — Per-axis MIDI mapping config
  osc/
    osc_handler.cpp/h   — OSC bundle output, UDP send, mutex-protected
    osc_translator.h    — Per-axis OSC mapping config
  hid/
    hid_translator.h    — Per-axis HID mapping (disabled)
    usb_hid.h           — TinyUSB HID (disabled)
  server/
    server.cpp/h        — HTTP endpoints (~22 routes), CORS, captive portal
    pipo-ws.cpp/h       — WebSocket handler: single-client, adaptive rate
    serve-files.hpp     — PipoFileServer: .gz auto-detect, ETag, chunked
    pipo-presets.h      — Preset scanner and endpoint logic
  utils/
    config.h/cpp        — Config class: load/save/apply/gather/validate
    pw-manager.hpp      — WiFi credential storage (NVS, LRU eviction)
  wifi/
    pipowifi.cpp/h      — WiFi state machine, event handlers, reconnect logic
web-config/              — Svelte 4 + Vite 5 + TypeScript frontend
  src/
    App.svelte          — Root: detects board type, renders variant UI
    pipoio.ts           — WS client: EventEmitter, queue serialization, reconnect
    services/config.ts  — Svelte stores: currentConfig, hasUnsavedChanges, etc.
    types.ts            — PipoConfig<T>, MidiConfig, OscConfig, InputSettings
    schema.ts           — Per-axis min/max/step/units for validation
    utils/config-validator.ts — Pre-save validation (null, NaN, empty string)
    lib/form/           — Generic form components (Number, Switch, Select, Range, etc.)
    lib/configs/        — ChannelSettings, MidiOutputSettings, OscOutputSettings
    lib/presets.svelte  — Preset selection UI
    lib/FloatingSaveButton.svelte — Validate → multipart upload → reboot if needed
  scripts/postbuild.js  — Injects sequential asset loader into index.html
configs/                 — Variant-specific default configs (analog/motion/range)
presets/                 — Per-variant preset libraries
test/                    — doctest C++ unit tests + Vitest WS integration tests
  stubs/                — ArduinoFake mocks (Preferences, WiFi, esp_log)
scripts/                 — build-web.sh, release.py, define-version.py, copy_default_config.py
```

---

## Architecture Overview

### Dual-Core RTOS Design

- **Core 1** (real-time): Main loop at 400Hz — sensor read → engine.update() → MIDI/OSC output
- **Core 0** (network/UI): WiFi state machine, WebSocket broadcast, LED control, battery monitor, button debounce

### RTOS Tasks

| Task                | Stack | Pri | Core | Period            | Purpose            |
| ------------------- | ----- | --- | ---- | ----------------- | ------------------ |
| Main loop (Arduino) | 8KB   | 1   | 1    | 2.5ms             | Sensor + engine    |
| websocketTask       | 3KB   | 2   | 0    | 40-300ms adaptive | WS telemetry       |
| wifiTask            | 4KB   | 3   | 0    | 500ms             | WiFi state machine |
| hwuiTask            | 2KB   | 1   | 0    | 10-20ms           | LED patterns       |
| battmonitorTask     | 2KB   | 1   | 0    | 500ms             | Battery sampling   |
| buttonTask          | 2KB   | 1   | 0    | 5ms               | Debounce + press   |

All tasks register with ESP32 watchdog (2s timeout).

### Sensor Pipeline

```
Raw reading
  → Sensor-specific filter (EMA / Median / LowPass+MA+Kalman)
  → apply_offset()
  → process_sensor_neutral_filter() (dynamic deadband)
  → value_ready [per-axis SensorDat]
  → Engine.update() per-axis:
      → invert / cyclic / over_out transforms
      → midi_processor() — CC or Note mode + sustain manager
      → osc_processor() — deduped, bundled UDP
      → (hid_processor disabled)
```

**Per-axis trigger_flags**: Each protocol (MIDI/OSC/HID) has independent trigger/untrigger flags allowing per-protocol entry/exit behavior.

### Config System

**4 required sections**: `engine`, `inputs`, `general`, `sensorconf`

**Lifecycle**: load → deserializeJson → validate → apply → (runtime edits) → gather → atomic save

**Atomic writes**: Write `.tmp` → verify size → `LittleFS.rename()` → update `last_config.txt`. Temp files cleaned on boot.

**Key class**: `Config` in `src/utils/config.h` — owns `current_config` JsonDocument.

- `apply(engine, osc)` pushes config to subsystems
- `gather(engine)` collects runtime state back to JsonDocument
- `validate_config()` checks all 4 sections present and non-null

**Variant configs**: `scripts/copy_default_config.py` copies variant-specific default + presets at buildfs time.

### Communication: WebSocket

Single-client enforced. TEXT frames only. Custom protocol:

**Inbound** (512B buffer): `command:payload`

- `config:path/to/key:value` — single config edit
- `configs:key:val\nkey:val` — batch edit
- `save:` — persist to flash
- `monitor:axis_name` — subscribe to axis telemetry

**Outbound** (256B buffer, adaptive rate):

```
fps,25.00,40.00
sensor,pitch,45.32,1
sensor,roll,-12.45,0
rssi,-65
logs,System ready
noteon,1,60,127
cc,1,14,64,0
```

**Adaptive rate**: 40ms (RSSI > -65) → 80ms → 200ms → 300ms (RSSI < -80). Doubled when BLE active.
**Memory guard**: Skip send if heap < 30KB or WS queue full.

### Communication: HTTP REST API

~22 endpoints. All have `Access-Control-Allow-Origin: *`. Key groups:

- **Config CRUD**: `/configs`, `/config`, `/active-config`, `/config-new`, `/config-copy`, `/config-delete`, `/config-rename`, `/save` (multipart upload)
- **Presets**: `/presets` (list), `/preset?name=X` (download), `/presets-refresh`
- **WiFi**: `/wifi-mode`, `/wifi-connect` (XOR-encoded pw), `/wifi-state`, `/wifi-networks`, `/wifi-start-scan`, `/wifi-disconnect`, `/wifi-forget`, `/wifi-clear-all`
- **Calibration**: `/offsetcal`, `/offsetcal-status`, `/offsetcal-list`, `/resetoffset`, `/setreference`
- **Monitor**: `/info`, `/logs`, `/conf-debug`, `/battlevel`, `/ping`, `/relative-mode`, `/pause`, `/resume`, `/reboot`
- **Captive Portal**: `/generate_204`, `/hotspot-detect.html`, `/success.html` → redirect to `/`

### WiFi State Machine

Event-driven via 13 Arduino WiFi event handlers → volatile shared_flags.

- Boot: APSTA mode → async scan → try last known STA (max 3 retries) → fallback to AP
- Password Manager (`pw-manager.hpp`): NVS storage, LRU eviction, `userWantsDisconnected` flag
- Network readiness: `pipoNetworkReady()` = STA connected OR AP configured + client IP assigned

### Web Frontend (Svelte)

- **State**: Svelte stores in `services/config.ts` — change detection via `JSON.stringify` every 300ms
- **WS Client** (`pipoio.ts`): EventEmitter, dead connection detection (3s), promise-based queue serialization, 25ms inter-operation delay
- **Save**: `FloatingSaveButton` validates → multipart POST → triggers reboot if output mode or PipoName changed
- **Build**: Vite → gzip (delete originals) → postbuild.js injects sequential loader → `scripts/build-web.sh` copies to `data/webpage/`
- **Dev**: `cd web-config && npm run dev` (port 5173) + `cd mock-server && npm run dev` (port 3000) with `VITE_STATIC_IP`

---

## Build System

### PlatformIO Environments

| Environment   | PIPO_TYPE | HW_REV | lib_extra_dirs      |
| ------------- | --------- | ------ | ------------------- |
| motion        | motion    | 10     | lib-variants/motion |
| motion_rev1_1 | motion    | 11     | lib-variants/motion |
| range         | range     | 10     | lib-variants/range  |
| range_rev1_1  | range     | 11     | lib-variants/range  |
| analog        | analog    | 10     | lib-variants/analog |
| analog_rev2   | analog    | 20     | lib-variants/analog |

Dev variants add `DISABLE_USB_COMM` + `CORE_DEBUG_LEVEL=5`.

### Common Build Commands

```bash
npm run build:motion_rev1_1      # Build firmware
npm run upload:motion_rev1_1     # Flash firmware
npm run build:fs:motion_rev1_1   # Build filesystem (web UI + configs + presets)
npm run upload:fs                # Flash filesystem
npm run build:web                # Build Svelte → data/webpage/
npm test                         # C++ unit tests (doctest)
npm run test:websockets          # Vitest WS integration
npm run serial                   # Serial monitor
npm run release                  # Full release (3 variants, merged bins, manifest)
```

### Partition Layout (8MB flash)

```
nvs     0x9000   20KB    NVS key-value storage
otadata 0xe000   8KB     OTA metadata
app0    0x10000  2MB     Main firmware
app1    0x210000 2MB     OTA update partition
spiffs  0x410000 3MB     LittleFS (configs, presets, web UI)
```

### Version: `git describe --tags` → `PIPO_FW_VERSION` macro (scripts/define-version.py)

### Release: `scripts/release.py` builds 3 variants → esptool merge → manifest.json → copies to pipo-site-vuepress

---

## Development Guidelines

### When Modifying the Sensor Pipeline

- Filters are per-variant in sensor subclasses — don't add filters in engine.cpp
- `SensorDat` struct owns all per-axis state — extend it for new per-axis features
- Trigger flags are per-protocol; set in `process_sensor_triggers()`, consumed in engine
- Main loop is 400Hz on Core 1 — keep processing under 2.5ms budget

### When Modifying Config

- Always maintain the 4-section schema (engine/inputs/general/sensorconf)
- After adding config fields: update `default.json` for ALL 3 variants in `configs/`
- After adding config fields: update TypeScript types in `web-config/src/types.ts`
- Config apply/gather must stay symmetric — every field in apply() needs a gather() counterpart
- No config schema versioning exists — consider backward compatibility carefully
- Test atomic save: power-loss during write must not corrupt config

### When Modifying WebSocket Protocol

- Inbound buffer is 512B, outbound is 256B — respect these limits
- Add new message types to both firmware (`pipo-ws.cpp`) and frontend (`pipoio.ts`)
- Frontend `_wrap()` queues WS commands — don't bypass it for new commands
- Adaptive rate is RSSI-driven; BLE doubles the delay
- Single-client enforced — new connections close old ones

### When Modifying HTTP Endpoints

- Add routes in `src/server/server.cpp`
- CORS headers added globally via `DefaultHeaders`
- For file operations: use atomic write pattern (`.tmp` → rename)
- For frontend integration: update Axios calls + add to mock-server routes

### When Modifying Web Frontend

- State lives in Svelte stores (`services/config.ts`) — don't duplicate state in components
- Change detection is `JSON.stringify` comparison — works for all nested changes
- New config fields need: TypeScript type + schema entry + form component binding
- Build output goes to `data/webpage/` via `build-web.sh`
- Test with mock server: `VITE_STATIC_IP=http://localhost:3000`
- Sequential loader in postbuild.js prevents FOUC on slow ESP32 WiFi

### When Modifying Build Configuration

- `lib_extra_dirs` in platformio.ini selects variant-specific sensor drivers
- `copy_default_config.py` runs at buildfs time — copies variant config + presets to `data/`
- Don't exceed 2MB firmware partition or 3MB LittleFS partition
- Total heap after setup: ~60KB free; ~50KB consumed by MIDI, ~50KB by WiFi, ~30KB by web server

### Memory Budget

```
Total SRAM: 520KB
After boot:  ~60KB free heap (typical)
Fragmentation: 38-43%

Major consumers:
  MIDI setup:    ~50KB
  WiFi setup:    ~50KB
  Web server:    ~30KB
  Config JSON:   5-15KB (unbounded JsonDocument — improvement needed)
  Per-axis maps: ~7.5KB (std::string in translators)
```

---

## Known Issues & Improvement Areas

1. **JsonDocument sizing** — Uses DynamicJsonDocument without size limits. Switch to sized allocation or StaticJsonDocument for predictable memory.
2. **No config schema versioning** — Adding/removing fields requires reflash. Consider a version field + migration logic.
3. **Per-axis std::string allocations** — MidiTranslator/OscTranslator/HidTranslator use heap strings. Consider fixed-size char arrays.
4. **No authentication** — All endpoints open, CORS `*`. Acceptable for local device but risky on shared networks.
5. **WiFi password XOR-only** — Not cryptographic. Fine for obfuscation over local network.
6. **HID disabled** — Code exists but commented out in engine.cpp. Needs TinyUSB HID endpoint activation.
7. **WebSocket single-value config edits** — `config:path:value` protocol is limited; `configs:` batch is a workaround.
8. **Change detection polling** — Frontend uses 300ms setInterval + JSON.stringify instead of reactive diffing.
9. **debugMonitorTask** — Commented out; causes stack crashes with DEBUG_HEAP on Core 1.
