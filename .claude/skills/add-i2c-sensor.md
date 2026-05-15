# add-i2c-sensor

Interactive wizard that integrates a new I2C sensor into the Pipo platform. Generates all required files: firmware driver, PlatformIO environment, config JSON, TypeScript types, and a manual-steps checklist.

---

## Phase 1 — Gather sensor information

Ask the user all of the following in **one message** so they can answer in one go:

```
To generate your Pipo sensor integration I need a few details:

1. Sensor model/name (e.g. "SHT31", "BME280", "APDS-9960")
2. Arduino/PlatformIO library name (exact name as in library registry, e.g. "adafruit/Adafruit SHT31 Library")
3. I2C address in hex (e.g. 0x44). If configurable, the default.
4. For each measurement axis/channel, provide:
   - Short axis key (snake_case, ≤12 chars — becomes the OSC address and MIDI channel label)
     e.g. `temp`, `humidity`, `lux`, `dist`, `presence`
   - Physical unit (e.g. °C, %, cm, lux, bool)
   - Sensor hardware minimum value (at rated range, e.g. -40 for temperature)
   - Sensor hardware maximum value (e.g. 125 for temperature)
   - Mode: continuous (float output) or trigger (binary on/off)?
   - Smoothing needed: none / light (EMA α=0.3) / heavy (moving-average 5-sample)
5. Max read rate in Hz (e.g. 10 for a slow humidity sensor, 100 for a fast distance sensor).
   This sets whether the sensor should skip frames to avoid blocking the 400Hz loop.
6. Any sensor-specific config parameters that should be runtime-adjustable?
   (e.g. gain, integration time, resolution, threshold). For each: name, type, default value.
7. Preferred variant name for the new PlatformIO environment (e.g. "my_env", "sht31_rev1").
   This becomes `npm run build:my_env` etc.
8. SDA/SCL pins: use defaults from HW_CONFIG.h (just say "default") or specify GPIO numbers.
```

Wait for the user's answers before proceeding.

---

## Phase 2 — Read reference files

Before generating code, read these files to use as implementation context. Do NOT skip this step.

- `src/sensors/input_sensor.h` — base Sensor class, SensorDat struct, all virtual methods
- `src/sensors/pipo_range_sensor.h` — simplest concrete sensor (2 axes, optional filters)
- `src/sensors/pipo_range_sensor.cpp` — measure_sensor() and config serialization pattern
- `src/utils/filters.h` — available filter classes (EMA, MovingAverage, LowPass, Kalman, StateBasedFilter)
- `src/sensors/sensors.h` — compile-time sensor selection pattern
- `src/sensors/sensors.cpp` — global instance pattern
- `platformio.ini` — existing env blocks to copy format from
- `configs/range_config.json` — config JSON structure (all 4 required sections)
- `web-config/src/types.ts` — TypeScript PipoConfig and InputSettings types
- `web-config/src/schema.ts` — axis schema pattern (min/max/step/units per axis)
- `web-config/src/App.svelte` — how board type drives which UI component renders

---

## Phase 3 — Generate all integration files

Use the user's answers and the reference files to generate each artifact below. Write each file directly. Be complete — no placeholder TODOs unless the user must fill in hardware-specific register details they didn't provide.

### 3.1 Sensor header — `src/sensors/{name}_sensor.h`

Pattern (adapt from pipo_range_sensor.h):

```cpp
#ifndef PIPO_{NAME}_SENSOR_H
#define PIPO_{NAME}_SENSOR_H

#include "sensors/input_sensor.h"
#include "utils/filters.h"
// Include the user's Arduino library here

class {Name}Sensor : public Sensor {
 public:
  {Name}Sensor();          // Constructor: declare all axes in sensor_dat map
  void init() override;    // I2C init + library begin()
  void setup() override;   // Configure measurement parameters, reset filters
  bool measure_sensor() override; // Read hardware, populate sensor_dat[axis].raw_value
                                  // Set reading_valid = true/false per axis
                                  // Return true if any value changed
  void set_sensor_config(JsonObject config, bool debug = false) override;
  JsonDocument get_sensor_config(bool debug = false) override;

 private:
  // Library object (e.g. Adafruit_SHT31 sht31;)
  // Filters: one per axis that needs smoothing
  // Sensor-specific runtime state
};

#endif
```

Key rules for the constructor:
- Declare every axis: `sensor_dat["axis_key"] = SensorDat();`
- Set `reading_valid = false` for all axes
- Binary/trigger axes should set `hold_mode = false`
- For slow sensors (< 50Hz): add a `_frame_skip` counter to skip frames in measure_sensor()

Key rules for `measure_sensor()`:
- Call the library read function
- On success: set `sensor_dat["key"].raw_value = reading; sensor_dat["key"].reading_valid = true;`
- Apply sensor-specific filter (EMA/MA): `sensor_dat["key"].value = filter.process(raw);`
  - If no filter: `sensor_dat["key"].value = sensor_dat["key"].raw_value;`
  - The base class handles the neutral filter (deadband) automatically — do not call it here
- On failure/invalid: set `reading_valid = false` (base class will handle hold_mode if enabled)
- Return `true` always if data is being produced (base class compares prev vs current)

Key rules for config serialization:
- `set_sensor_config()` reads sensor-specific params from JsonObject (e.g. gain, resolution)
- `get_sensor_config()` returns a JsonDocument with those same params
- The `inputs` section (deadband, lmax, lmin, etc.) is handled by the base class — do not duplicate it here

### 3.2 Sensor implementation — `src/sensors/{name}_sensor.cpp`

Pattern (adapt from pipo_range_sensor.cpp):

```cpp
#include "sensors/{name}_sensor.h"
#include <Arduino.h>

{Name}Sensor::{Name}Sensor() {
  // Axis declarations already done in header constructor body — move here if preferred
}

void {Name}Sensor::init() {
  Wire.begin(SDA_PIN, SCL_PIN);   // Use pins from HW_CONFIG or user-specified
  if (!lib.begin(I2C_ADDR)) {
    log_e("{Name} sensor not found at 0x{ADDR}");
    return;
  }
  log_i("{Name} sensor initialized");
}

void {Name}Sensor::setup() {
  // Apply any library configuration: lib.setGain(...), lib.setResolution(...) etc.
  // Reset filters if any
}

bool {Name}Sensor::measure_sensor() {
  // --- Frame skip for slow sensors (delete if sensor can keep up with 400Hz) ---
  // _frame_counter++;
  // if (_frame_counter < _frame_divisor) return false;
  // _frame_counter = 0;

  // --- Read sensor ---
  float raw = lib.readAxis();  // Replace with actual library call
  if (/* error condition */ false) {
    sensor_dat["key"].reading_valid = false;
    return true;
  }

  // --- Apply filter (if any) ---
  sensor_dat["key"].raw_value = raw;
  sensor_dat["key"].value = ema_filter.process(raw); // or = raw if no filter
  sensor_dat["key"].reading_valid = true;

  return true;
}

void {Name}Sensor::set_sensor_config(JsonObject config, bool debug) {
  // Example: if (config["gain"].is<int>()) _gain = config["gain"].as<int>();
}

JsonDocument {Name}Sensor::get_sensor_config(bool debug) {
  JsonDocument doc;
  // Example: doc["gain"] = _gain;
  return doc;
}
```

### 3.3 Sensor registration — patches to existing files

**`src/sensors/sensors.h`** — add after the last `#ifdef` block:
```cpp
#ifdef PIPO_{NAME}
#include "sensors/{name}_sensor.h"
extern {Name}Sensor input_sensor;
#endif
```

**`src/sensors/sensors.cpp`** — add after the last `#ifdef` block:
```cpp
#ifdef PIPO_{NAME}
#include "sensors/{name}_sensor.h"
{Name}Sensor input_sensor;
#endif
```

**`src/HW_CONFIG.h`** — add to the type-string block:
```cpp
#elif defined(PIPO_{NAME})
#define PIPO_TYPE "{name}"
```

### 3.4 PlatformIO environment — new block in `platformio.ini`

```ini
[env:{variant_name}]
extends = env:release
build_flags =
  ${env:release.build_flags}
  -DPIPO_{NAME}=1
  -DHW_REV=11
  '-DUSB_PRODUCT="Pipo-{Name}"'
lib_extra_dirs = lib-variants/{name}
lib_deps =
  ${env:release.lib_deps}
  {arduino_library_name}
```

Note: create directory `lib-variants/{name}/` (can be empty if library is sourced from registry).

### 3.5 Default config — `configs/{name}_config.json`

Use range_config.json as structural template. Populate the `inputs` section with one entry per axis using the user's lmin/lmax values. Continuous axes: `"mode": false, "th_mode": false`. Trigger axes: `"mode": true, "th_mode": true`.

```json
{
  "general": {
    "Wifi_mode": "AP",
    "OSC_ENA": true,
    "OSC_PORT": 8000,
    "OSC_IP": "0.0.0.0",
    "MidiEnabled": true,
    "PipoName": "{name}",
    "BLE_enabled": false,
    "USB_MIDI_enabled": false,
    "relative_mode": false,
    "pipo_relative_mode": false
  },
  "inputs": {
    "{axis_key}": {
      "deadband": 0.5,
      "offset": 0.0,
      "inverted": false,
      "lmax": {user_max},
      "lmin": {user_min},
      "mode": false,
      "th_mode": false,
      "cyclic": false,
      "over_out": false
    }
  },
  "sensorconf": {
    // sensor-specific params go here, matching set_sensor_config() keys
  },
  "engine": {
    "engine-midi": {
      "{axis_key}": {
        "channel": 1, "tl_mode": 0, "enabled": true,
        "min_input": {user_min}, "max_input": {user_max},
        "cc_nb": 14, "cc_max": 127, "cc_min": 0, "hires": false,
        "scaleType": "chromatic", "pattern": "up", "rootNote": 60,
        "sustain": 500, "nbOfNotes": 12, "current_scale": [], "velocity": 100
      }
    },
    "engine-osc": {
      "{axis_key}": {
        "enabled": true, "mode_raw": false,
        "osc_max": 1.0, "osc_min": 0.0,
        "osc_addr": "/{name}/{axis_key}"
      }
    }
  }
}
```

### 3.6 TypeScript types — additions to `web-config/src/types.ts`

Add a sensor-config type if there are runtime-configurable sensor params:
```typescript
export type {Name}SensorConfig = {
  // one field per sensorconf param, matching JSON keys
  // example: gain: number;
};

export type PipoConfig{Name} = PipoConfig<{Name}SensorConfig>;
```

If no sensor-specific params, a type alias is still useful:
```typescript
export type PipoConfig{Name} = PipoConfig<Record<string, never>>;
```

### 3.7 Schema additions — `web-config/src/schema.ts`

Add per-axis schema entries so the web UI knows valid ranges and units:
```typescript
// {Name} sensor axes
{axis_key}: { min: {lmin}, max: {lmax}, step: {step}, units: "{unit}" },
```

Step is typically `(max - min) / 100` rounded to a sensible precision.

---

## Phase 4 — Print the integration checklist

After generating all files, print this checklist for the user to complete manually:

```
## {Name} Sensor Integration Checklist

### Files generated (verify they were written):
- [ ] src/sensors/{name}_sensor.h
- [ ] src/sensors/{name}_sensor.cpp
- [ ] configs/{name}_config.json

### Files patched (verify the additions):
- [ ] src/sensors/sensors.h — PIPO_{NAME} block added
- [ ] src/sensors/sensors.cpp — global instance added
- [ ] src/HW_CONFIG.h — PIPO_TYPE string added
- [ ] platformio.ini — [env:{variant_name}] block added
- [ ] web-config/src/types.ts — type added
- [ ] web-config/src/schema.ts — axis schema added

### Manual steps:
- [ ] mkdir -p lib-variants/{name}  (even if empty)
- [ ] Implement measure_sensor() — replace stub library calls with actual {arduino_library_name} API
- [ ] Wire the sensor to the board (SDA={sda_pin}, SCL={scl_pin}, VCC=3.3V)
- [ ] Add a web UI component in web-config/src/lib/configs/ for any sensor-specific config
      (copy OscOutputSettings.svelte as a template)
- [ ] Update App.svelte to render your new config component when board type == "{name}"
- [ ] Run: npm run build:{variant_name}    — verify firmware compiles
- [ ] Run: npm run upload:{variant_name}   — flash to device
- [ ] Run: npm run build:fs:{variant_name} && npm run upload:fs  — flash web UI
- [ ] Open the Pipo web UI and confirm axes appear and respond to sensor movement
- [ ] Test OSC output with a DAW or OSC monitor app
- [ ] Test MIDI output
- [ ] Add presets/ folder:  mkdir -p presets/{name}
- [ ] Copy scripts/copy_default_config.py handling for new variant if needed

### Calibration:
- [ ] Use the web UI offset calibration to zero out any DC bias
- [ ] Adjust lmin/lmax in the config to match actual sensor output range
- [ ] Tune deadband (0.3-1.0%) to taste for your use case
```

---

## Notes for Claude when running this skill

- Keep generated C++ idiomatic to the project style: `log_i()`, `log_e()` not `Serial.print()`
- Do not add error recovery beyond what the existing sensors do — keep it consistent
- The base class Sensor::update() handles: neutral filter, offset, bounds check, engagement, trigger flags. The subclass must NOT call these — only set raw_value, value, reading_valid.
- The `lib_extra_dirs` line in platformio.ini points to a local library folder. If the user's library is in the PlatformIO registry, use `lib_deps` instead (or both)
- Always generate the full default config JSON — never omit the `engine` section, it must have both `engine-midi` and `engine-osc` entries for every axis
