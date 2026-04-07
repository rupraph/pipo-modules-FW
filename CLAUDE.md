# Pipo-Single — AI Context

ESP32-S3 firmware (PlatformIO/Arduino) + Svelte 4 web config UI for wireless sensor-to-MIDI/OSC controllers. 3 device variants (motion/range/analog) sharing one codebase with compile-time switching.

## Build & Flash

```bash
npm run build:motion_rev1_1      # Build firmware (current default)
npm run upload:motion_rev1_1     # Flash firmware via USB
npm run build:fs:motion_rev1_1   # Build filesystem (web UI + configs + presets)
npm run upload:fs                # Flash filesystem
npm run build:web                # Build Svelte frontend only
npm run serial                   # Serial monitor (115200 baud)
npm test                         # C++ unit tests (doctest)
npm run test:websockets          # WebSocket integration tests (Vitest)
```

Other variants: `motion`, `range`, `range_rev1_1`, `analog`, `analog_rev2`. Append to any command (e.g., `npm run build:analog_rev2`).

## Coding Conventions

- C++ firmware: Arduino-style, ESP-IDF APIs where needed, ArduinoJson for serialization
- Use `log_i()`, `log_w()`, `log_e()` for firmware logging (ESP log macros)
- Frontend: Svelte 4 + TypeScript strict, Axios for HTTP, custom EventEmitter WS client
- Config changes must update: firmware structs + `default.json` (all 3 variants in `configs/`) + TypeScript types (`web-config/src/types.ts`) + web UI
- Sensor pipeline runs at 400Hz on Core 1 — keep it fast, no blocking calls
- WebSocket buffers: 512B inbound, 256B outbound — don't exceed
- Atomic file writes always: `.tmp` → verify → rename
- Heap budget: ~60KB free after boot, fragmentation 38-43%

## Project Map

```
src/
  main.cpp              — Boot sequence, RTOS task creation
  engine.cpp/h          — Output multiplexer (MIDI/OSC/HID per-axis)
  HW_CONFIG.h           — Pin defs, variant constants, debug flags
  hw_ui.cpp/h           — LEDs (LEDC/WS2812), buttons, battery monitor
  shared_flags.h        — Volatile inter-task flags (no queues for perf)
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
    lib/form/           — Generic form components (Number, Switch, Select, Range)
    lib/configs/        — ChannelSettings, MidiOutputSettings, OscOutputSettings
    lib/presets.svelte  — Preset selection UI
    lib/FloatingSaveButton.svelte — Validate → multipart upload → reboot if needed
  scripts/postbuild.js  — Injects sequential asset loader into index.html
configs/                 — Variant-specific default configs (analog/motion/range)
presets/                 — Per-variant preset libraries
test/                    — doctest C++ unit tests + Vitest WS integration tests
  stubs/                — ArduinoFake mocks (Preferences, WiFi, esp_log)
scripts/                 — build-web.sh, release.py, define-version.py, copy_default_config.py
mock-server/             — Express mock for frontend dev (port 3000)
```

## Architecture

### Dual-Core RTOS

- **Core 1** (real-time): Main loop at 400Hz — sensor read → engine.update() → MIDI/OSC output
- **Core 0** (network/UI): WiFi state machine, WebSocket broadcast, LED control, battery monitor, button debounce

| Task | Stack | Pri | Core | Period | Purpose |
|------|-------|-----|------|--------|---------|
| Main loop (Arduino) | 8KB | 1 | 1 | 2.5ms | Sensor + engine |
| websocketTask | 3KB | 2 | 0 | 40-300ms adaptive | WS telemetry |
| wifiTask | 4KB | 3 | 0 | 500ms | WiFi state machine |
| hwuiTask | 2KB | 1 | 0 | 10-20ms | LED patterns |
| battmonitorTask | 2KB | 1 | 0 | 500ms | Battery sampling |
| buttonTask | 2KB | 1 | 0 | 5ms | Debounce + press |

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
```

Sensor types & filters:
- **Motion** (ICM-20948): EMA(0.7) — 12 axes: accXYZ, roll/pitch/yaw, magXYZ, gyroXYZ
- **Range** (VL53L4CX/VL53L1): LowPass + MovingAvg + Kalman — 1 axis: dist
- **Analog**: MedianFilter(5) analog, MedianFilter(10) touch — 16 axes: A01-A08, T1-T8

### Config System

**4 required sections**: `engine`, `inputs`, `general`, `sensorconf`

**Lifecycle**: load → deserializeJson → validate → apply → (runtime edits) → gather → atomic save

**Atomic writes**: Write `.tmp` → verify size → `LittleFS.rename()` → update `last_config.txt`. Temp files cleaned on boot.

**Key class**: `Config` in `src/utils/config.h` — owns `current_config` JsonDocument.
- `apply(engine, osc)` pushes config to subsystems
- `gather(engine)` collects runtime state back to JsonDocument
- `validate_config()` checks all 4 sections present and non-null

**Variant configs**: `scripts/copy_default_config.py` copies variant-specific default + presets at buildfs time.

**Filesystem layout (LittleFS)**:
```
/default.json, /last_config.txt
/configs/*.json (+ .tmp for atomic writes)
/presets/*.json (with metadata header: preset.name, preset.description)
/webpage/ (web UI + .gz compressed assets)
```

### WebSocket Protocol

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
noteon,1,60,127
cc,1,14,64,0
```

Adaptive rate: 40ms (RSSI > -65) → 80ms → 200ms → 300ms (RSSI < -80). Doubled when BLE active. Skip send if heap < 30KB or WS queue full.

### HTTP REST API

~22 endpoints. All have `Access-Control-Allow-Origin: *`.

- **Config CRUD**: `/configs`, `/config`, `/active-config`, `/config-new`, `/config-copy`, `/config-delete`, `/config-rename`, `/save` (multipart)
- **Presets**: `/presets`, `/preset?name=X`, `/presets-refresh`
- **WiFi**: `/wifi-mode`, `/wifi-connect` (XOR-encoded pw), `/wifi-state`, `/wifi-networks`, `/wifi-start-scan`, `/wifi-disconnect`, `/wifi-forget`, `/wifi-clear-all`
- **Calibration**: `/offsetcal`, `/offsetcal-status`, `/offsetcal-list`, `/resetoffset`, `/setreference`
- **Monitor**: `/info`, `/logs`, `/conf-debug`, `/battlevel`, `/ping`, `/relative-mode`, `/pause`, `/resume`, `/reboot`

### WiFi State Machine

Event-driven via Arduino WiFi event handlers → volatile shared_flags.
- Boot: APSTA mode → async scan → try last known STA (max 3 retries) → fallback to AP
- Password Manager (`pw-manager.hpp`): NVS storage, LRU eviction, `userWantsDisconnected` flag
- `pipoNetworkReady()` = STA connected OR AP configured + client IP assigned

### Web Frontend

- **State**: Svelte stores in `services/config.ts` — change detection via `JSON.stringify` every 300ms
- **WS Client** (`pipoio.ts`): EventEmitter, dead connection detection (3s), promise queue serialization, 25ms inter-operation delay
- **Save**: FloatingSaveButton validates → multipart POST `/config-upload` → triggers reboot if output mode or PipoName changed
- **Build**: Vite → gzip → postbuild.js sequential loader → `scripts/build-web.sh` copies to `data/webpage/`
- **Dev**: `cd web-config && npm run dev` + `cd mock-server && npm run dev` with `VITE_STATIC_IP=http://localhost:3000`

## Build System

### PlatformIO Environments

| Environment | PIPO_TYPE | HW_REV | lib_extra_dirs |
|-------------|-----------|--------|----------------|
| motion | motion | 10 | lib-variants/motion |
| motion_rev1_1 | motion | 11 | lib-variants/motion |
| range | range | 10 | lib-variants/range |
| range_rev1_1 | range | 11 | lib-variants/range |
| analog | analog | 10 | lib-variants/analog |
| analog_rev2 | analog | 20 | lib-variants/analog |

### Partition Layout (8MB flash)

```
nvs     0x9000   20KB    NVS key-value storage
otadata 0xe000   8KB     OTA metadata
app0    0x10000  2MB     Main firmware
app1    0x210000 2MB     OTA update partition
spiffs  0x410000 3MB     LittleFS (configs, presets, web UI)
```

### Version: `git describe --tags` → `PIPO_FW_VERSION` macro via `scripts/define-version.py`

## Testing

- C++ tests in `test/` use doctest + ArduinoFake stubs in `test/stubs/`
- Frontend dev: `cd web-config && npm run dev` + `cd mock-server && npm run dev`
- Set `VITE_STATIC_IP=http://localhost:3000` for mock device

## Development Checklists

### When Modifying Config
1. Update firmware struct/handling in `src/utils/config.h`
2. Update `default.json` for ALL 3 variants in `configs/`
3. Update TypeScript types in `web-config/src/types.ts`
4. Update schema in `web-config/src/schema.ts` if new min/max/units needed
5. Update web UI component bindings
6. Ensure apply/gather symmetry

### When Modifying WebSocket Protocol
1. Update firmware handler in `src/server/pipo-ws.cpp`
2. Update frontend client in `web-config/src/pipoio.ts`
3. Update mock server in `mock-server/src/ws.ts`
4. Respect buffer limits: 512B in, 256B out

### When Adding HTTP Endpoints
1. Add route in `src/server/server.cpp`
2. Add Axios call in frontend
3. Add mock route in `mock-server/src/routes.ts`

## Known Issues

1. **JsonDocument sizing** — DynamicJsonDocument without limits (5-15KB). Switch to sized allocation.
2. **No config schema versioning** — Adding/removing fields requires reflash.
3. **Per-axis std::string heap allocations** — Translators use heap strings; consider fixed-size arrays.
4. **No authentication** — All endpoints open, CORS `*`.
5. **HID disabled** — Code exists but commented out in engine.cpp.
6. **Change detection polling** — Frontend uses 300ms setInterval + JSON.stringify.
