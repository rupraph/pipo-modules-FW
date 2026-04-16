# Sensor Trigger Truth Table

## Continuous Mode (mode=0)

The trigger/untrigger flags tell the engine when a sensor _enters_ or _exits_ the active range, enabling note-on/note-off and OSC start/stop events.

| Row | in_range_prev | in_range | hold_mode | Action               | Trigger | Untrigger | Value output                       |
| --- | ------------- | -------- | --------- | -------------------- | ------- | --------- | ---------------------------------- |
| 1   | false         | false    | false     | Stay out of range    | —       | —         | Current (no data or abs_max)       |
| 2   | false         | false    | true      | Stay out, hold value | —       | —         | Held (last in-range value)         |
| 3   | false         | true     | false     | **Enter range**      | **YES** | —         | Current                            |
| 4   | false         | true     | true      | **Enter range**      | **YES** | —         | Current                            |
| 5   | true          | true     | false     | Stay in range        | —       | —         | Current                            |
| 6   | true          | true     | true      | Stay in range        | —       | —         | Current                            |
| 7   | true          | false    | false     | **Exit range**       | —       | **YES**   | Current (abs_max or out-of-bounds) |
| 8   | true          | false    | true      | **Exit range**       | —       | **YES**   | Held (last in-range value)         |

### Key behaviors

- **Trigger** (rows 3-4): Sets `trigger_flags` for all protocols → engine sends note-on / OSC start
- **Untrigger** (rows 7-8): Sets `untrigger_flags` for all protocols → engine sends note-off / OSC stop
- **Hold mode** (rows 2, 8): When `hold_mode=true` and sensor is out of range, `.value` is reverted to `value_prev_measure` (the value before `measure_sensor()` ran), so downstream processing (offset, neutral filter, engine) continues to see the last in-range value
- **No-data** (row 1 without hold): Sensor reports current reading which may be `abs_max` or filtered out-of-bounds value

## Threshold Mode (mode=1)

Uses `bool_value` (computed from `value_ready` vs `lmin`/`lmax`) instead of `in_range`:

| th_mode    | bool_value = true when                      |
| ---------- | ------------------------------------------- |
| 0 (basic)  | `value_ready > lmin`                        |
| 1 (window) | `value_ready > lmin AND value_ready < lmax` |

| bool_value_prev | bool_value | Action              |
| --------------- | ---------- | ------------------- |
| false           | true       | Set trigger flags   |
| true            | false      | Set untrigger flags |
| same            | same       | No action           |

## Where in_range is set

| Sensor       | in_range_set_by_sensor | How in_range is determined                                                        |
| ------------ | ---------------------- | --------------------------------------------------------------------------------- |
| Range (ToF)  | **true**               | Hardware validity + `raw_value` within `[lmin, lmax)` — set in `measure_sensor()` |
| Motion (IMU) | false                  | Auto-computed by base class: `value_ready > lmin && value_ready < lmax`           |
| Analog (ADC) | false                  | Auto-computed by base class: `value_ready > lmin && value_ready < lmax`           |
