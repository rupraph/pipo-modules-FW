import type { Schema } from "./types";

export const schema: Schema = {
  motion: {
    accX: {
      label: "X-axis acceleration",
      unit: "m/s²",
      min: -16,
      max: 16,
    },
    accY: {
      label: "Y-axis acceleration",
      unit: "m/s²",
      min: -16,
      max: 16,
    },
    accZ: {
      label: "Z-axis acceleration",
      unit: "m/s²",
      min: -16,
      max: 16,
    },
    pitch: {
      label: "Pitch",
      unit: "°",
      min: -90,
      max: 90,
    },
    roll: {
      label: "Roll",
      unit: "°",
      min: -180,
      max: 180,
    },
    yaw: {
      label: "Yaw",
      unit: "°",
      min: -180,
      max: 180,
    },
  },
  range: {
    dist: {
      label: "Distance",
      unit: "mm",
      min: 0,
      max: 5000,
    },
  },
  analog: {
    A1: {
      label: "Analog 1",
      unit: "V",
      min: 0,
      max: 3.3,
    },
    A2: {
      label: "Analog 2",
      unit: "V",
      min: 0,
      max: 3.3,
    },
    A3: {
      label: "Analog 3",
      unit: "V",
      min: 0,
      max: 3.3,
    },
    A4: {
      label: "Analog 4",
      unit: "V",
      min: 0,
      max: 3.3,
    },
    A5: {
      label: "Analog 5",
      unit: "V",
      min: 0,
      max: 3.3,
    },
    A6: {
      label: "Analog 6",
      unit: "V",
      min: 0,
      max: 3.3,
    },
    T1: {
      label: "Touch 1",
      unit: "-",
      min: 0,
      max: 200000,
    },
    T2: {
      label: "Touch 2",
      unit: "-",
      min: 0,
      max: 200000,
    },
    T3: {
      label: "Touch 3",
      unit: "-",
      min: 0,
      max: 200000,
    },
    T4: {
      label: "Touch 4",
      unit: "-",
      min: 0,
      max: 200000,
    },
    T5: {
      label: "Touch 4",
      unit: "-",
      min: 0,
      max: 200000,
    },
    T6: {
      label: "Touch 4",
      unit: "-",
      min: 0,
      max: 200000,
    },
  },
  unknown: { "": { label: "", unit: "", min: 0, max: 0 } },
};
