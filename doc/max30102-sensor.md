# MAX30102 PPG Sensor — Implementation Reference

## Overview

The `max30102` variant adds photoplethysmography (PPG) heart-rate sensing to Pipo using the Maxim MAX30102 pulse oximeter chip. It emits Red + IR LED light into skin, measures reflected light with a photodetector, and extracts the pulsatile (AC) blood-flow signal from the baseline (DC) tissue absorption.

- **Chip**: MAX30102 (Red + IR LEDs only — no Green)
- **I2C address**: 0x57, 400 kHz fast mode
- **Library**: SparkFun MAX3010x (`lib-variants/max30102/SparkFun_MAX3010x_Sensor_Library/`)
- **Heart rate algorithm**: Custom per-channel pipeline (DC removal → AC filter → envelope → normalize) in `Max30102Sensor`
- **Pins**: Same as motion/range — I2C on GPIO 17/18, shared pin layout
- **Status**: No SpO2 in current implementation

---

## 8 Axes

Axes are grouped into three categories in the web UI:

### Raw PPG Signals

| Axis          | Source              | Range      | Unit | Description                          |
| ------------- | ------------------- | ---------- | ---- | ------------------------------------ |
| `ir_raw`      | `getFIFOIR()`       | 0–262,143  | raw  | Raw 18-bit IR photodetector reading  |
| `red_raw`     | `getFIFORed()`      | 0–262,143  | raw  | Raw 18-bit Red photodetector reading |
| `temperature` | `readTemperature()` | -40 to +85 | °C   | Die temperature (read every ~5s)     |

### Preconditioned (DC-removed + Bandpass Filtered)

These are the primary musical control axes — the pulsatile AC signals respond to heartbeat. Values are **normalized to [0, 1]** for consistent behavior regardless of sensor placement or skin perfusion.

| Axis     | Source                                     | Range     | Unit | Description                                                   |
| -------- | ------------------------------------------ | --------- | ---- | ------------------------------------------------------------- |
| `ir_ac`  | DC removal → AC LPF → envelope → normalize | 0.0–1.0   | norm | IR pulsatile component (0=diastole, 1=systole, 0.5=no signal) |
| `red_ac` | DC removal → AC LPF → envelope → normalize | 0.0–1.0   | norm | Red pulsatile component (same normalization)                  |
| `ir_dc`  | Slow low-pass (fc≈0.5Hz) on raw            | 0–262,143 | raw  | IR DC baseline (tissue + venous absorption)                   |
| `red_dc` | Slow low-pass (fc≈0.5Hz) on raw            | 0–262,143 | raw  | Red DC baseline                                               |

### Computed

| Axis     | Source                                          | Range | Unit | Description                       |
| -------- | ----------------------------------------------- | ----- | ---- | --------------------------------- |
| `hr_bpm` | Normalized IR rising-edge detector + 4-beat avg | 0–255 | BPM  | Heart rate, 4-beat moving average |

---

## Signal Processing Pipeline

Sensor is configured at **1600 Hz** (max for Red+IR 2-LED mode), `sample_average=1` (no internal averaging), `pulse_width=69 µs` (shortest). Each 400 Hz Pipo loop iteration drains ~4 samples from the FIFO.

IR and Red are processed **independently** through identical per-channel pipelines (no shared state):

```
400 Hz loop iteration:
  particleSensor_.check()                           // non-blocking FIFO drain
  while (particleSensor_.available()):
    ir  = getFIFOIR()                                // paired, same-sample read
    red = getFIFORed()
    nextSample()

    For each channel (IR, Red):
      ┌─ raw.value = raw                            // Raw (pass-through)
      │
      ├─ dc  = slowLPF(raw, fc≈0.5Hz)               // Tracks baseline drift only
      │        (first sample primes filter)
      │
      ├─ ac  = lightLPF(raw - dc, fc≈8Hz)           // Removes high-freq noise
      │
      ├─ envelope tracking:                          // Sliding-window equivalent (O(1) memory)
      │     env_max = max(ac, env_max * 0.99928)     //   instant attack, decay over ~2s
      │     env_min = min(ac, env_min * 0.99928)
      │
      ├─ normalized = (ac - env_min) / (env_max - env_min)
      │     clamped to [0, 1], mid=0.5 when amplitude < 50 counts
      │
      └─ ac.value = normalized                      // [0,1] placement-independent output

    ┌─ if ir_normalized crosses above 0.6 (rising edge):
    │     beat detected → BPM = 60000 / delta_ms
    └─ hr_bpm.value = avg(last 4 BPM readings)

  Every ~5s: temperature.value = readTemperature()   // blocks ~100ms
```

### Why `getFIFOIR`/`getFIFORed` (not `getIR`/`getRed`)

`getIR()`/`getRed()` call `safeCheck(250)` which **blocks up to 250 ms** waiting for new data — incompatible with the 400 Hz real-time pipeline. The FIFO variants are non-blocking and return paired readings from the same sensor sample, which is essential for correct AC/DC separation.

### Filter Details

| Filter           | Implementation                                                                       | Purpose                                                                           |
| ---------------- | ------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------- |
| DC Removal       | 1st-order IIR low-pass (fc≈0.5Hz, α≈0.002 at 1600Hz), per-channel                    | Tracks baseline drift (respiration, pressure) while rejecting heartbeat (0.5–3Hz) |
| AC Smoothing     | 1st-order IIR low-pass (fc≈8Hz, α≈0.03 at 1600Hz), per-channel                       | Removes high-frequency noise, preserves heartbeat harmonics                       |
| Envelope Tracker | Peak/valley detectors with instant attack + exponential decay (τ≈0.87s), per-channel | Sliding-window min/max equivalent — adapts within one heartbeat, O(1) memory      |
| Normalization    | `(ac - env_min) / (env_max - env_min)` clamped to [0,1]                              | Placement-independent output: 0=diastole, 1=systole, 0.5=no confident signal      |
| Heart Rate       | Rising edge of normalized IR crossing 0.6 (hysteresis: must drop below 0.4 first)    | Amplitude-independent beat detection                                              |
| BPM Averaging    | Ring buffer of 4 most recent inter-beat intervals → average                          | Smooths beat-to-beat variability                                                  |

All axes set their `NeutralFilter` deadband to **0** (pass-through) since all filtering is handled by the per-channel pipeline upstream.

### Finger Detection

When raw IR value < 5,000 (no finger on sensor), `reading_valid` is set to `false` for all axes except `temperature`. Uses raw IR directly (not the DC estimate) for immediate response — no filter lag.

---

## Sensor Configuration (`sensorconf`)

```json
{
  "led_brightness": 31,
  "sample_rate": 1600,
  "pulse_width": 69,
  "adc_range": 16384
}
```

| Field            | Range                              | Default            | Notes                                                                                           |
| ---------------- | ---------------------------------- | ------------------ | ----------------------------------------------------------------------------------------------- |
| `led_brightness` | 0–255                              | 31 (0x1F = 6.4 mA) | 0xFF = 50 mA max. Stored in memory, applied to HW in `setup()`. Runtime changes require reboot. |
| `sample_rate`    | 50, 100, 200, 400, 800, 1000, 1600 | 1600               | 1600 is max for Red+IR 2-LED mode. Changes require reboot.                                      |
| `pulse_width`    | 69, 118, 215, 411 µs               | 69                 | Shorter pulse = faster sampling but shorter sensing range. Changes require reboot.              |
| `adc_range`      | 2048, 4096, 8192, 16384            | 16384              | Full 18-bit range. Changes require reboot.                                                      |

`sample_average` is hardcoded to **1** (no internal averaging, raw samples only). Not exposed in config.

---

## Key Design Decisions

1. **No SpO2**: The Maxim `spo2_algorithm.cpp` is included in the library but not used. SpO2 requires 100-sample buffers at exactly 25 Hz with strict sample timing — conflicts with the 1600 Hz/400 Hz real-time pipeline. Can be added later as a background computation on Core 0.

2. **No Green LED**: The MAX30102 only has Red + IR LEDs. Green would require switching to MAX30105.

3. **Custom per-channel DSP pipeline**: Replaces the SparkFun `heartRate.cpp` global-state DSP (which shared a single FIR buffer between IR and Red, corrupting both signals). Each channel now has independent state: DC filter, AC filter, envelope tracker, and normalized output. All filters are 1st-order IIR (O(1) memory) — no circular buffers needed.

4. **No ambient light axis**: Could be derived from IR DC when LEDs are off, but explicitly excluded to keep axis count manageable.

5. **No live I2C writes for config**: `set_sensor_config()` only stores values in member variables. The actual hardware register writes happen in `setup()`. This avoids a boot crash where `config.apply()` (called before `init()`) would try to write I2C registers on an uninitialized bus.

6. **Polled FIFO, no interrupt pin**: The INT pin is not used. `check()` polls the FIFO read/write pointers — simpler, avoids pin allocation.

7. **Temperature read throttled to 5 s**: `readTemperature()` blocks for ~100 ms (I2C poll loop). Reading every ~2,000 loop iterations prevents pipeline disruption.

8. **Normalized AC output [0, 1]**: `ir_ac` and `red_ac` are placement-independent: 0=diastole, 1=systole, 0.5=no confident signal. Envelope adapts within one heartbeat via instant-attack/decay peak detectors. The old approach output raw ADC differences (~50–5,000 depending on placement), making it unusable as a control source.

---

## Boot Sequence Interaction

A critical constraint: `Config::apply()` is called **before** `input_sensor.init()`/`setup()` in `main.cpp`. This means `set_sensor_config()` must never touch hardware registers (no I2C, no GPIO that hasn't been configured). It only stores config values in member variables. The actual hardware setup happens in:

```
config.apply()      → stores led_brightness_, sample_rate_, etc. in memory (no I2C)
input_sensor.init() → Wire.begin() + particleSensor_.begin()
input_sensor.setup()→ particleSensor_.setup(led_brightness_, ...) ← applies to HW
```

---

## Memory Footprint

| Component                                    | Size      |
| -------------------------------------------- | --------- |
| 2 × `PPGChannel` (filters + envelope state)  | ~48 B     |
| `LowPassFilter` internal state (4 instances) | ~32 B     |
| Rate buffer (4 × uint8_t)                    | 4 B       |
| **Total (new pipeline)**                     | **~84 B** |

Down from ~300 B (old pipeline: `heartRate.cpp` globals + 32-sample FIR buffer + DC estimator state). Negligible impact on the ~60 KB free heap budget.

---

## Build & Flash

```bash
npm run build:max30102        # Build firmware
npm run upload:max30102       # Flash firmware via USB
npm run build:fs:max30102     # Build filesystem (web UI + config + presets)
npm run upload:fs             # Flash filesystem
npm run full:max30102         # Full cycle: web → fs → erase → flash
```

### PlatformIO Environment

```ini
[env:max30102]
board_build.variant = pipo_max30102
extends = env:release
build_flags = -DPIPO_MAX30102=1 -DHW_REV=10
lib_extra_dirs = lib-variants/max30102
```

---

## Source Files

| File                                                          | Purpose                                                                         |
| ------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| `src/sensors/max30102_sensor.h`                               | `Max30102Sensor` class — inherits `Sensor`, 8 axes, config accessors            |
| `src/sensors/max30102_sensor.cpp`                             | Init, setup, measure (FIFO drain + pipeline), config get/set                    |
| `lib-variants/max30102/SparkFun_MAX3010x_Sensor_Library/src/` | SparkFun driver (`MAX30105`) + Maxim algorithms (`heartRate`, `spo2_algorithm`) |
| `configs/max30102_config.json`                                | Default config with all 8 axes in engine/inputs/sensorconf                      |
| `presets/max30102/`                                           | MIDI heartbeat + OSC PPG preset examples                                        |
| `web-config/src/lib/configs/max30102-channels.svelte`         | Channel selector UI with 3 category tabs                                        |
| `variants/pipo_max30102/`                                     | Pin mapping (same as pipo_range)                                                |
