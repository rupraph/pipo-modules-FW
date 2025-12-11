# Sensor Data Pipeline

This document describes the processing stages for sensor data in the PIPO system.

## Processing Stages

The sensor data flows through the following stages:

```
┌─────────────┐
│ raw_value   │  Pure sensor reading (no processing)
└──────┬──────┘
       │
       │ Sensor-specific filtering (in individual sensor classes)
       │ e.g., mag/accel filtering, low-pass filters
       ▼
┌─────────────┐
│ value       │  After sensor-specific filtering
└──────┬──────┘
       │
       │ apply_offset() - Subtract calibrated offset
       ▼
┌─────────────┐
│ value_offset│  After offset applied (used for UI display)
└──────┬──────┘
       │
       │ process_sensor_neutral_filter() - Dynamic deadband filter
       ▼
┌─────────────┐
│ value_ready │  Final filtered value (used by engine for MIDI/OSC/HID)
└─────────────┘
``
```
