# BLE MIDI Layer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace lathoub BLE-MIDI + MIDI Library + vendored NimBLE 1.4.1 with our own BLE-MIDI transport on NimBLE-Arduino 2.5.1 that sends only as fast as the link allows, batches messages, and receives Program Change.

**Architecture:** A pure, header-only codec (`ble_midi_codec.h`: packet encoder, bounds-safe parser, outbound queue with coalescing and a drop policy) unit-tested natively, plus a thin NimBLE glue (`midiBLE.cpp`) that owns the GATT server, a Core-0 sender task paced by the connection interval, and a static inbound Program Change queue. Producers (engine on Core 1) only append to the queue under a spinlock; they never touch BLE.

**Tech Stack:** NimBLE-Arduino 2.5.1 (`lib_deps`), Arduino-ESP32 3.3.12 (pioarduino 55.03.12), FreeRTOS, doctest (native env, `-std=gnu++11`).

**Spec:** the user's requirements in this conversation (2026-10-23): inbound MIDI (Program Change to switch configs, soon); connection interval and MTU tuned directly in NimBLE; several messages batched per notification, sent only as fast as the BLE stack can take them; maintained NimBLE 2.x with the 3.3 core's BLE memory opt-in; very light on heap; drop when overloaded.

## Global Constraints

- NimBLE-Arduino pinned to `2.5.1`; the vendored `lib/NimBLE-Arduino`, `lib/BLE-MIDI`, `lib/MIDI Library`, `lib/ArduinoBLE` are removed.
- No heap allocation on the send path; all buffers static and fixed-size.
- Nothing BLE-related runs on Core 1: producers only lock, copy 3 bytes, unlock.
- Codec header must compile under the native test env (`-std=gnu++11`, no Arduino/NimBLE includes).
- Channels are 1–16 at the `MidiBLEsend*` API (same as `USBMIDI`), 0–15 inside status bytes.
- Preferred MTU 128 (payload ≤ 125 bytes → one 256-byte mbuf per notification).
- Connection request 7.5–15 ms (units 6–12), latency 0, supervision timeout 3 s (300); Apple-compliant fallback 15/15 ms after 3 s if the interval is still > 15 ms.
- NimBLE pool sizes/roles come from the core's prebuilt `sdkconfig.h` and cannot be changed with `-D` flags; do not add role/connection flags.
- Firmware not uploaded by us; user flashes and reports.

## Review Focus

1. Host disconnects mid-burst, then reconnects → no stale messages from the previous session are sent (queue cleared on disconnect and on subscribe).
2. Link slower than producers (e.g. 30 ms interval, 12 axes moving) → CCs coalesce to latest value, Note Offs are never lost while Note Ons exist to evict, nothing blocks Core 1.
3. Malformed or truncated inbound packet → no out-of-bounds read, no crash, later valid messages in the same packet still parsed.
4. MIDI clock / SysEx from a DAW in the same packet as a Program Change → Program Change still delivered.
5. Default MTU (23) before or without MTU exchange → batches limited to 20-byte packets, still correct.

---

### Task 1: Codec — encoder and parser

**Files:**
- Create: `src/midi/ble_midi_codec.h`
- Test: `test/test_ble_midi/ble_midi.test.cpp`

**Interfaces:**
- Produces: `struct BleMidiMsg { uint8_t status, d1, d2; };`, `uint8_t bleMidiMsgLen(uint8_t status)` (2 for 0xC0/0xD0, else 3), `size_t bleMidiEncode(const BleMidiMsg* msgs, size_t n, uint16_t timeMs, uint8_t* out, size_t cap)` (returns bytes written, 0 if it does not fit), `template <class F> void bleMidiParse(const uint8_t* data, size_t len, F onMessage)` (calls `onMessage(status, d1, d2)` for channel-voice messages), constant `kBleMidiMaxPacket = 125`.

- [ ] **Step 1: Write failing tests**: encode `{B0 07 64},{C0 03}` at t=0x1ABC → `B5 BC B0 07 64 BC C0 03`; encode into too-small buffer → 0; encode→parse round trip; parse: single PC, two timestamped messages, running status with and without timestamp, clock between messages (PC still delivered), SysEx then PC, SysEx-continuation packet then PC, truncated message (no emit; a byte past `len` must not be read), empty / 1-byte / header without bit 7, orphan data bytes, trailing timestamp.
- [ ] **Step 2: Run** `./scripts/pio.sh test -e native -f test_ble_midi` → FAIL (header missing).
- [ ] **Step 3: Implement** encoder (header `0x80|((t>>7)&0x3F)`, then per message `0x80|(t&0x7F)` + bytes, one timestamp value for the packet) and parser state machine: after a timestamp byte, a status byte sets running status (real-time ≥0xF8 skipped without touching state, 0xF0 enters SysEx, 0xF7 leaves it, other system common clears running status and skips its data bytes); data bytes with no running status or inside SysEx are skipped; a message whose data bytes run past `len` or hit a byte with bit 7 is not emitted.
- [ ] **Step 4: Run** tests → PASS.

### Task 2: Codec — outbound queue

**Files:**
- Modify: `src/midi/ble_midi_codec.h`
- Test: `test/test_ble_midi/ble_midi.test.cpp`

**Interfaces:**
- Produces: `class BleMidiOutQueue` with `kOrderedCap = 32`, `kLatestCap = 32`; `bool push(uint8_t status, uint8_t d1, uint8_t d2)` (false = dropped); `size_t takeBatch(BleMidiMsg* out, size_t maxMsgs, size_t maxBytes)` (`maxBytes` includes the header byte; each message costs `len + 1`); `void clear()`; `bool empty() const`; `struct Stats { uint32_t droppedOrdered, droppedLatest, evictedNoteOn; }; const Stats& stats() const`.

- [ ] **Step 1: Write failing tests**: CC same key coalesces to last value; different CC numbers / channels stay separate; pitch bend coalesces per channel; notes keep order and never coalesce; batch returns ordered (notes/PC) before latest-value (CC/PB/aftertouch); `maxBytes=9` returns two 3-byte messages and leaves the third; ordered FIFO full + Note On → dropped and counted; FIFO full + Note Off → oldest Note On evicted (counted), Note Off appended last; Note On velocity 0 counts as Note Off; FIFO full of Note Offs + Note Off → dropped; duplicate queued Note Off is not queued twice; latest slots full → new key dropped, existing key still updates; round-robin: after taking CC1, a re-dirtied CC1 does not starve CC2; `clear()` empties.
- [ ] **Step 2: Run** → FAIL.
- [ ] **Step 3: Implement** with a linear ordered array (append, memmove to remove head/evict) and a slot array `{status, key, d1, d2, used, dirty}` keyed by `(status, d1)` for 0xA0/0xB0 and `(status)` for 0xD0/0xE0; round-robin start index for dirty slots.
- [ ] **Step 4: Run** → PASS.

### Task 3: NimBLE glue, library swap, build

**Files:**
- Modify: `src/midi/midiBLE.h`, `src/midi/midiBLE.cpp`, `platformio.ini` (`lib_deps` of `env:release`, `env:custom`, `env:dev`)
- Delete: `lib/NimBLE-Arduino`, `lib/BLE-MIDI`, `lib/MIDI Library`, `lib/ArduinoBLE`

**Interfaces:**
- Consumes: Task 1–2 codec.
- Produces (unchanged for `midi_io`): `midiBLESetup()`, `MidiBLEsendCC/NoteOn/NoteOff(int, int, int channel1to16)`, `MidiBLEsendPitchBend(int signedValue, int channel)`; new `bool midiBLEReadProgramChange(uint8_t* channel1to16, uint8_t* program)` (non-blocking).

- [ ] **Step 1:** Setup: `NimBLEDevice::init(name)`, `setMTU(128)`, `setSecurityAuth(true,false,false)` (same as before), server with `advertiseOnDisconnect(true)`, MIDI service `03B80E5A-EDE8-4B33-A751-6CE34EC4C700`, characteristic `7772E5DB-3868-4112-A1A9-F2669D106BF3` READ|WRITE|WRITE_NR|NOTIFY, max_len 128; advertise service UUID in adv data and name in scan response; static inbound queue (8 × 3 bytes); sender task 3072 B, prio 2, Core 0.
- [ ] **Step 2:** Callbacks: connect → store handle/interval, reset payload to 20, `BTconnected=true`, request 6–12/0/300; MTU change → payload `min(mtu-3,125)`; params update → store interval; subscribe → clear queue, ready = notifications enabled, wake task; disconnect → ready=false, clear queue, `BTconnected=false`, log stats; write → parse, push Program Change to inbound queue (drop when full), clear the characteristic value.
- [ ] **Step 3:** Sender task: idle on task notification while not ready; otherwise wake every connection interval, run the 3 s Apple fallback once, then send up to 3 packets: refill the task-local pending batch from the queue (spinlock), skip the tick if `os_msys_num_free() < 8`, encode with `millis()`, `notify(buf, len, connHandle)`; on failure keep the pending batch for the next tick.
- [ ] **Step 4:** Producers: return unless ready; build status from channel; push under spinlock.
- [ ] **Step 5:** Swap libraries, then `npm run build:dev_motion_rev1_1` and `npm run build:motion_rev1_1` → SUCCESS; `nm` on the ELF shows `_setBleLibraryInUse` (memory opt-in linked).

### Task 4: Documentation

**Files:**
- Create: `docs/ble-midi.md`
- Modify: `CLAUDE.md` (project map + task table)

- [ ] **Step 1:** Document how the layer works (threads, pacing, batching, drop policy, inbound, connection/MTU, heap, limits, what callers must do), in the style of `docs/wifi-battery-saving.md`.
- [ ] **Step 2:** Run `./scripts/pio.sh test -e native` → all suites PASS.
