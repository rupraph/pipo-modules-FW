# BLE MIDI Message Batching Implementation

## Overview

This document describes the BLE MIDI message batching optimization implemented to improve BLE-MIDI performance by reducing packet overhead and taking advantage of BLE connection intervals.

## Problem Statement

Previously, each MIDI message (NoteOn, NoteOff, ControlChange) was sent individually over BLE:

- **One BLE packet per MIDI message** (3-5 bytes per packet)
- Each packet triggered a separate `notify()` call
- BLE connection intervals (typically 7.5-30ms) were underutilized
- High overhead from BLE stack for each small packet

## Solution

Implemented message batching to accumulate multiple MIDI messages in the BLE-MIDI transport buffer and send them together in a single BLE packet.

### Key Benefits

1. **Reduced BLE Packets**: 5-10x reduction in packet count during high-throughput scenarios
2. **Better Interval Utilization**: Send 20+ bytes per interval instead of 3-5 bytes
3. **Lower Latency**: Reduced processing overhead in the BLE stack
4. **Spec Compliant**: BLE-MIDI specification explicitly supports multiple messages per packet

## Implementation Details

### 1. Low-Level BLE Functions (`midiBLE.cpp/h`)

Added two new functions to control the BLE-MIDI transport buffer:

```cpp
void MidiBLEbeginBatch()
```

- Starts a batch transmission
- Messages accumulate in the `mTxBuffer` (64 bytes)
- Calls `MidiBle.beginTransmission(MIDI_NAMESPACE::InvalidType)`

```cpp
void MidiBLEendBatch()
```

- Flushes all accumulated messages in one BLE packet
- Calls `MidiBle.endTransmission()`

### 2. MIDI I/O Layer (`midi_io.cpp/h`)

Added batch control methods that conditionally handle BLE batching:

```cpp
void midi_io::beginBatch()
void midi_io::endBatch()
```

These methods:

- Only affect BLE when enabled (`config.general_config["BLEEnabled"]`)
- USB MIDI continues to send immediately (no batching needed)
- Provide a clean API for higher-level code

### 3. Engine Integration (`engine.cpp`)

Batching is applied at the main update loop level:

**Main Update Loop:**

```cpp
void Engine::update() {
  // Begin batch at start of update cycle
  midiio.beginBatch();
  midiio.manage_sustain();

  // Process all sensors (may send multiple MIDI messages)
  for (auto const& pair : sensor_dat) {
    midi_processor(...);  // May send CC, NoteOn, NoteOff
  }

  // Flush all messages in one BLE packet
  midiio.endBatch();
}
```

**Additional Batching Points:**

- `sendAllNotesOff()`: Batches multiple note-off messages
- `sendHiResControlChange()`: Batches MSB and LSB CC messages together

## Buffer Capacity

- **Buffer Size**: 64 bytes (from `BLEMIDI_Settings.h`)
- **Capacity**: ~16-20 CC messages or ~10-12 Note messages per batch
- **Auto-flush**: Buffer automatically flushes when full

## Message Timing

All messages in a batch share the same BLE connection interval timing, which is acceptable because:

- The messages originate from the same sensor update cycle
- Timing granularity of ~7.5-15ms is sufficient for most use cases
- Individual timestamps are still included in the BLE-MIDI packet format

## Performance Expectations

### Before Batching

- 1 sensor with 3 axes → 3 BLE packets per update
- High-res CC (2 messages) → 2 BLE packets
- Multiple notes → N BLE packets

### After Batching

- All messages from one update cycle → 1 BLE packet
- Typical reduction: **5-10x fewer BLE packets**
- Reduced CPU overhead in BLE stack

## Testing Recommendations

1. **Verify Functionality**: Test that all MIDI messages are received correctly
2. **Monitor Latency**: Ensure no perceptible latency increase
3. **High-Throughput Test**: Send rapid CC changes on multiple axes simultaneously
4. **Connection Stability**: Verify stable BLE connection under load

## Backward Compatibility

- Changes are transparent to USB MIDI (no behavioral change)
- BLE behavior only changes when `BLEEnabled` is true
- No changes to MIDI protocol or message format
- Compatible with all BLE-MIDI receivers

## Future Optimizations

Consider these additional improvements:

1. **Lower Connection Interval**: Request 7.5ms minimum interval in `OnConnected()`

   ```cpp
   server->updateConnParams(handle, 6, 12, 0, 500);
   ```

2. **Adaptive Batching**: Dynamically adjust batch size based on message rate

3. **Priority Messages**: Optionally send time-critical messages immediately

## Files Modified

- `src/midi/midiBLE.h` - Added batch function declarations
- `src/midi/midiBLE.cpp` - Implemented batch functions
- `src/midi/midi_io.h` - Added batch control API
- `src/midi/midi_io.cpp` - Implemented batch control and updated functions
- `src/engine.cpp` - Integrated batching in main update loop

## References

- BLE-MIDI Specification v1.0a
- Library: `lib/BLE-MIDI/src/BLEMIDI_Transport.h`
- NimBLE Arduino: `lib/NimBLE-Arduino/`
