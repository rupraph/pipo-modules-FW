# Pipo-Single — Copilot Workspace Instructions

## Project

ESP32-S3 firmware (PlatformIO/Arduino) + Svelte web config UI for wireless sensor-to-MIDI/OSC controllers. 3 variants: motion (IMU), range (ToF), analog (ADC+touch).

## Build & Flash

```bash
npm run build:motion_rev1_1      # Build firmware (current default)
npm run upload:motion_rev1_1     # Flash firmware via USB
npm run build:fs:motion_rev1_1   # Build filesystem (web UI + configs)
npm run upload:fs                # Flash filesystem
npm run build:web                # Build Svelte frontend only
npm run serial                   # Serial monitor (115200 baud)
npm test                         # C++ unit tests (doctest)
npm run test:websockets          # WebSocket integration tests (Vitest)
```

## Coding Conventions

- C++ firmware: Arduino-style, ESP-IDF APIs where needed, ArduinoJson for serialization
- Use `log_i()`, `log_w()`, `log_e()` for firmware logging (ESP log macros)
- Frontend: Svelte 4 + TypeScript strict, Axios for HTTP, custom EventEmitter WS client
- Config changes must update: firmware structs + default.json (all 3 variants) + TypeScript types + web UI
- Sensor pipeline runs at 400Hz on Core 1 — keep it fast, no blocking calls
- WebSocket buffers: 512B inbound, 256B outbound — don't exceed
- Atomic file writes always: `.tmp` → verify → rename

## Key Architecture

- Dual-core: Core 1 (sensor+engine real-time), Core 0 (WiFi/WS/UI)
- Config: 4 sections (engine/inputs/general/sensorconf), ArduinoJson, LittleFS
- Inter-task: volatile shared_flags (no queues for perf), mutex on OSC UDP only
- Heap budget: ~60KB free after boot, fragmentation 38-43%

## Testing

- C++ tests in `test/` use doctest + ArduinoFake stubs in `test/stubs/`
- Frontend dev: `cd web-config && npm run dev` + `cd mock-server && npm run dev`
- Set `VITE_STATIC_IP=http://localhost:3000` for mock device
