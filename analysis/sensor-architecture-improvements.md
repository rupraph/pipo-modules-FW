# Sensor Architecture — Remaining Improvements

Improvements identified during the in_range/hold_mode generalization and architecture review. Quick wins (const ref, direct field access) were implemented. These remain.

## Done (for reference)

- `get_sensor_dat_map()` returns `const&` instead of copying the entire map every frame
- `Engine::update()` reads `SensorDat` fields directly — eliminates ~16 hash lookups per axis per frame
- `pipo_socket.cpp` WS monitor reads fields directly
- `in_range`, `hold_mode` generalized to base `Sensor` class via `SensorDat` fields
- Replaced `std::map<string,float> held_values` with `value_prev_measure` field on `SensorDat`

## Medium effort

### Split SensorDat into config vs live state

`SensorDat` mixes config fields (deadband, offset, inverted, lmax, lmin, mode, th_mode, over_out, cyclic, hold_mode) with live per-frame state (raw_value, value, value_ready, value_prev, bool_value, trigger_flags, etc.). Splitting into `AxisConfig` + `AxisState` would:
- Make it clear what's set at config time vs updated at 400Hz
- Allow config to be stored in a separate flat structure not touched by the hot loop
- Reduce cache line pollution (config fields sit next to hot floats)

### Translator maps use string keys

`Miditranslators`, `Osctranslators` are `unordered_map<string, T>`. Every `find()` in the engine loop hashes a string. Could use the same index/enum as a future flat sensor array.

## Large effort

### Replace string-keyed sensor_dat map with flat array + enum

`sensor_dat` is `unordered_map<string, SensorDat>`. At 400Hz with 12 axes (motion), every access is a string hash + comparison. A flat `SensorDat[]` array indexed by an enum (`enum Axis { PITCH, ROLL, YAW, ACCX, ... }`) would:
- Eliminate all hash overhead on the hot path
- Improve cache locality (contiguous memory)
- Require an axis name ↔ index mapping for config/WS/HTTP (lookup table, only used on slow paths)

This is the highest-impact change but touches every file that accesses `sensor_dat` by string key (sensors, engine, config, WS, HTTP handlers).

### Per-axis std::string heap allocations in translators

`MidiTranslator` and `OscTranslator` store `std::string` members (OSC addresses, etc.) that allocate on the heap per-axis. For fixed-format addresses, these could be `char[]` or generated on-the-fly to reduce heap fragmentation. Lower priority — these are set at config time, not per-frame.
