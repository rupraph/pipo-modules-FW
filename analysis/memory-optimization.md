# Memory Optimization Analysis - ESP32-S3 PiPo Project

**Date**: December 16, 2025  
**Target**: ESP32-S3 running PlatformIO Arduino  
**Goal**: Identify and reduce heap consumption, minimize fragmentation

---

## Executive Summary

**Total Identified Savings: ~10-20KB heap**

- Critical issues: JsonDocument unbounded allocations, large static buffers, per-axis translator overhead
- Heap fragmentation caused by: std::string, std::vector dynamic allocations
- Pre-allocation opportunities identified across translators, config system, websockets

---

## 🔴 Critical Memory Issues

### 1. JsonDocument Objects - Unbounded Dynamic Allocation

**Location**: `src/utils/config.h:37-38`

```cpp
JsonDocument current_config;  // Unbounded heap allocation
JsonDocument general_config;  // Unbounded heap allocation
```

**Problem**:

- ArduinoJson v7 `JsonDocument` allocates heap dynamically based on content
- Can consume 5-15KB depending on config complexity
- Causes fragmentation on every load/save cycle

**Impact**: ~5-15KB + fragmentation

**Solutions**:

- ✅ **SAFE**: Use `StaticJsonDocument<SIZE>` with explicit sizing
- ⚠️ **RISKY**: Pass by reference instead of by value in functions

**Temporary JsonDocuments in functions**:

- `engine.cpp`: `Engine::get_config()` - returns by value
- `engine.cpp`: Multiple temporary JsonDocuments in `set_config()`
- `config.cpp`: Multiple function-local JsonDocuments

---

### 2. MIDI CC State Array - 2048 bytes

**Location**: `src/midi/midi_io.h:35`

```cpp
uint8_t lastcc[16][128];  // 16 channels × 128 CCs = 2KB
```

**Problem**:

- Stores last CC value for ALL 16 channels × 128 controllers
- Most projects use only 2-4 channels and 10-20 CCs

**Impact**: 2KB static allocation

**Solutions**:

- ✅ **SAFE**: Reduce to realistic usage: `uint8_t lastcc[4][32]` = 128 bytes (if only 4 channels used)
- ⚠️ **RISKY**: Use `std::unordered_map<uint16_t, uint8_t>` (only for used CCs, but adds fragmentation)

---

### 3. WebSocket Buffers - 2304 bytes

**Location**: `src/server/pipo_socket.h:52-55`

```cpp
char inMsg[2048];   // 2KB input buffer
char outMsg[256];   // 256 bytes output buffer
```

**Problem**:

- `inMsg` is oversized for actual client messages (~50-100 bytes typical)
- Client analysis shows max message size ~200-300 bytes

**Impact**: 2.3KB static allocation

**Solutions**:

- ✅ **SAFE**: Reduce `inMsg` to 512 bytes (saves 1.5KB)
- ⚠️ Consider increasing `outMsg` if needed for logs

---

## 🟡 Per-Axis Translator Memory (Moderate Impact)

### Memory per 16 Axes (Analog variant)

| Component           | Stack/BSS  | Heap        | Total      | Notes                                 |
| ------------------- | ---------- | ----------- | ---------- | ------------------------------------- |
| MidiTranslator × 16 | ~400B      | ~1.6KB      | ~2KB       | `vector<int> current_scale` + strings |
| OscTranslator × 16  | ~700B      | ~512B       | ~1.2KB     | `string osc_addr`                     |
| HidTranslator × 16  | ~100B      | ~1KB        | ~1.1KB     | 2 `string` members                    |
| SensorDat × 16      | ~1.3KB     | -           | ~1.3KB     | Struct per axis                       |
| Filter maps         | ~400B      | ~640B       | ~1KB       | MedianFilter vectors                  |
| Map overhead        | ~800B      | -           | ~800B      | unordered_map overhead                |
| **TOTAL**           | **~3.7KB** | **~3.75KB** | **~7.5KB** | Per-axis allocations                  |

**Additional One-Time:**

- Static scale/arpeggio maps: ~6.2KB (shared across all instances)

---

### Translator Details

#### MidiTranslator (~85-125 bytes per instance)

**Location**: `src/midi/midi_translator.h:21-38`

```cpp
bool enabled               // 1 byte
int channel, cc_nb         // 8 bytes
int tl_mode                // 4 bytes
string pattern             // ~32 bytes HEAP! ❌
string scaleType           // ~32 bytes HEAP! ❌
int rootNote, nbOfNotes    // 8 bytes
vector<int> current_scale  // 24B + (25 notes × 4B = 100B HEAP!) ❌
float sustain              // 4 bytes
int cc_max, cc_min         // 8 bytes
bool hires                 // 1 byte
int velocity               // 4 bytes
```

**Heap allocations per instance**: ~164 bytes
**× 16 axes = ~2.6KB heap fragmentation**

---

#### OscTranslator (~70-80 bytes per instance)

**Location**: `src/osc/osc_translators.h:9-34`

```cpp
bool enabled, mode_raw     // 2 bytes
float osc_max, osc_min     // 8 bytes
string osc_addr            // ~32 bytes HEAP! ❌
```

**Heap allocations per instance**: ~32 bytes
**× 16 axes = ~512 bytes heap fragmentation**

---

#### HidTranslator (~70 bytes per instance)

**Location**: `src/hid/hid_translator.h:9-50`

```cpp
bool enabled, stroke_mode  // 2 bytes
string addr                // ~32 bytes HEAP! ❌
string addr2               // ~32 bytes HEAP! ❌
float last_value           // 4 bytes
```

**Heap allocations per instance**: ~64 bytes
**× 16 axes = ~1KB heap fragmentation**

---

### SensorDat Struct (~80 bytes per instance)

**Location**: `src/sensors/input_sensor.h:21-69`

**All stack/static allocated - no fragmentation** ✅

```cpp
// Config: 7 floats + 5 bools = ~35 bytes
// Live data: 6 floats + 2 bools = ~26 bytes
// Trigger flags: 2 structs × 3 bools = 6 bytes
// StateBasedFilter: ~20 bytes
```

**Total**: ~80 bytes × 16 axes = ~1.3KB (well-structured)

---

### Filter Objects

#### Motion Sensor (12 axes)

**Location**: `src/sensors/acc_sensor.h:72-75`

```cpp
unordered_map<string, EMAFilter> filter_map = {
  {"roll", EMAFilter(0.7)},   // Per entry: 32B key + 12B filter + 8B overhead
  {"pitch", EMAFilter(0.7)},  // = ~52 bytes × 12 axes = 624 bytes
  // ... 12 total
};
```

**EMAFilter**: 12 bytes (clean, no heap) ✅  
**Problem**: String keys + map overhead = ~40 bytes wasted per entry

---

#### Analog Sensor (8 axes)

**Location**: `src/sensors/analog_sensor.h:65-68`

```cpp
unordered_map<string, MedianFilter> filter_map = {
  {"A01", MedianFilter(5)},  // Actually uses windowSize=20 by default!
  // ... 8 total
};
```

**MedianFilter**:

- `vector<float> buffer` with capacity 20 = **80 bytes HEAP per filter!** ❌
- `size_t windowSize`: 4-8 bytes
- **Total**: ~88 bytes per filter × 8 axes = ~704 bytes heap

**Problem**: MedianFilter constructor shows windowSize=20, allocates vector on heap

---

## 🟢 Static Scale Data (Low Priority)

**Location**: `src/midi/midi_translator.cpp:379-404`

```cpp
const unordered_map<string, vector<int>> scales;     // ~4KB
const unordered_map<string, vector<int>> arpegios;   // ~1.5KB
const unordered_map<string, vector<int>> intervals;  // ~700B
```

**Total**: ~6.2KB (one-time, shared across all instances)

**Note**: These are static const, allocated once at startup. Low priority for optimization.

---

## ✅ Safe, No-Risk Optimizations (Pre-allocation Focus)

### Priority 1: Reduce WebSocket Input Buffer

**File**: `src/server/pipo_socket.h:48-52`

**Change**:

```cpp
const int inMaxLen = 512;   // Reduced from 2048
char inMsg[512];            // Reduced from 2048
```

**Savings**: 1.5KB  
**Risk**: None (client sends max ~200-300 byte messages)  
**Impact**: Immediate static memory savings

---

### Priority 2: Pre-size JsonDocuments with StaticJsonDocument

**Files**: `src/utils/config.h`, `src/engine.cpp`

**Change**:

```cpp
// config.h
StaticJsonDocument<6144> current_config;  // Pre-allocate 6KB
StaticJsonDocument<512> general_config;   // Pre-allocate 512B

// engine.cpp - in functions that return JsonDocument
StaticJsonDocument<2048> j;  // Instead of bare JsonDocument
```

**Savings**: 5-10KB heap + eliminates fragmentation  
**Risk**: Need to verify config size fits (check with serializeJson())  
**Impact**: Major fragmentation reduction

---

### Priority 3: Replace std::string with Fixed Buffers in Translators

**Safe changes (no API changes needed)**:

**OscTranslator** (`src/osc/osc_translators.h`):

```cpp
// Change from:
string osc_addr = "/";

// To:
char osc_addr[64] = "/";
```

**Savings**: ~512 bytes heap × 16 axes  
**Risk**: None (addresses are short)  
**Impact**: Eliminates per-axis string allocations

**HidTranslator** (`src/hid/hid_translator.h`):

```cpp
// Change from:
string addr = "a";
string addr2 = "b";

// To:
char addr[16] = "a";
char addr2[16] = "b";
```

**Savings**: ~1KB heap × 16 axes  
**Risk**: None (HID key names are short)  
**Impact**: Eliminates per-axis string allocations

---

### Priority 4: Don't Store Full Scale in MidiTranslator

**File**: `src/midi/midi_translator.h:37`

**Change**: Remove member variable, compute on-the-fly

```cpp
// Remove this:
// vector<int> current_scale;

// In get_note() method, use local variable:
int get_note(float value, float min_input, float max_input) {
  vector<int> scale = generate_full_scale(...);  // Local, freed immediately
  // ... use scale
}
```

**Savings**: 100 bytes × 16 axes = ~1.6KB heap  
**Risk**: Minimal (scale generation is fast)  
**Impact**: Reduces per-axis allocations

---

### Priority 5: Reduce MIDI CC Array Dimensions

**File**: `src/midi/midi_io.h:35`

**If only 4 channels used**:

```cpp
// Change from:
uint8_t lastcc[16][128];  // 2KB

// To:
uint8_t lastcc[4][32];    // 128 bytes
```

**Savings**: 1.9KB  
**Risk**: Medium (need to verify channel/CC usage)  
**Impact**: Significant static savings

---

### Priority 6: Pre-reserve Map Capacity

**File**: `src/engine.h:42-44`

**Add to Engine constructor**:

```cpp
Engine() {
  Miditranslators.reserve(16);  // Pre-allocate for 16 axes
  Osctranslators.reserve(16);
  HID_translators.reserve(16);

  for (const auto& axis : input_sensor.get_sensor_dat_map()) {
    Miditranslators[axis.first] = MidiTranslator();
    Osctranslators[axis.first] = OscTranslator();
    HID_translators[axis.first] = HidTranslator();
  }
}
```

**Savings**: Minimal direct, but reduces fragmentation from map resizing  
**Risk**: None  
**Impact**: Prevents map rehashing/reallocation during runtime

---

## 🔧 Implementation Roadmap

### Phase 1: Immediate, Zero-Risk Changes

1. ✅ Reduce `inMsg` buffer to 512 bytes
2. ✅ Pre-reserve map capacity in Engine constructor

**Time**: 5 minutes  
**Savings**: 1.5KB  
**Risk**: None

### Phase 2: String → Fixed Buffer Conversions

1. ✅ OscTranslator: `string osc_addr` → `char osc_addr[64]`
2. ✅ HidTranslator: `string addr/addr2` → `char addr[16]/addr2[16]`

**Time**: 30 minutes (update getters/setters)  
**Savings**: ~1.5KB heap  
**Risk**: Low (need to update string operations)

### Phase 3: StaticJsonDocument Sizing

1. ⚠️ Analyze actual config sizes with serialization
2. ✅ Convert `JsonDocument` → `StaticJsonDocument<SIZE>` in config.h
3. ✅ Update function signatures to pass by reference

**Time**: 1-2 hours (testing required)  
**Savings**: 5-10KB + fragmentation  
**Risk**: Medium (need correct sizing)

### Phase 4: Advanced Optimizations

1. ⚠️ Remove `vector<int> current_scale` from MidiTranslator
2. ⚠️ Reduce MIDI CC array dimensions
3. ⚠️ Replace filter maps with array-based indexing

**Time**: 2-4 hours  
**Savings**: 2-3KB  
**Risk**: Medium to High (functional changes)

---

## 📊 Total Expected Savings Summary

| Optimization                         | Savings     | Risk   | Phase |
| ------------------------------------ | ----------- | ------ | ----- |
| Reduce inMsg buffer                  | 1.5KB       | None   | 1     |
| String → Fixed buffers (translators) | 1.5KB       | Low    | 2     |
| StaticJsonDocument                   | 5-10KB      | Medium | 3     |
| Remove current_scale vector          | 1.6KB       | Medium | 4     |
| Reduce MIDI CC array                 | 1.9KB       | Medium | 4     |
| **TOTAL**                            | **11-16KB** | -      | -     |

**Fragmentation Reduction**: Significant (eliminates most per-operation allocations)

---

## 🎯 Recommended Starting Point

**Begin with Phase 1 + 2** (Zero to Low Risk):

- Reduce inMsg buffer
- Pre-reserve maps
- Convert translator strings to fixed buffers

**Total Savings**: ~3KB  
**Time**: 30-45 minutes  
**Risk**: Minimal  
**Benefit**: Immediate heap savings + reduced fragmentation

---

## 📝 Notes

- ESP32-S3 heap fragmentation is cumulative - small allocations add up
- Pre-allocation at startup is better than dynamic allocation during runtime
- Fixed-size buffers eliminate fragmentation entirely for string storage
- Map pre-reservation prevents costly rehashing operations

**Last Updated**: December 16, 2025
