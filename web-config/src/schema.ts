import type { Schema } from "./types";

export const schema: Schema = {
  motion: {
    accX: {
      label: "X-axis acceleration",
      unit: "m/s²",
      min: -10,
      max: 10,
    },
    accY: {
      label: "Y-axis acceleration",
      unit: "m/s²",
      min: -10,
      max: 10,
    },
    accZ: {
      label: "Z-axis acceleration",
      unit: "m/s²",
      min: -10,
      max: 10,
    },
    pitch: {
      label: "Pitch",
      unit: "°",
      min: -180,
      max: 180,
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
      unit: "cm",
      min: 0,
      max: 100,
    },
  },
  analog: {
    A1: {
      label: "Analog 1",
      unit: "V",
      min: 0,
      max: 5,
    },
    A2: {
      label: "Analog 2",
      unit: "V",
      min: 0,
      max: 5,
    },
    A3: {
      label: "Analog 3",
      unit: "V",
      min: 0,
      max: 5,
    },
    A4: {
      label: "Analog 4",
      unit: "V",
      min: 0,
      max: 5,
    },
    A5: {
      label: "Analog 5",
      unit: "V",
      min: 0,
      max: 5,
    },
    A6: {
      label: "Analog 6",
      unit: "V",
      min: 0,
      max: 5,
    },
    T1: {
      label: "Temperature 1",
      unit: "°C",
      min: -40,
      max: 125,
    },
    T2: {
      label: "Temperature 2",
      unit: "°C",
      min: -40,
      max: 125,
    },
    T3: {
      label: "Temperature 3",
      unit: "°C",
      min: -40,
      max: 125,
    },
    T4: {
      label: "Temperature 4",
      unit: "°C",
      min: -40,
      max: 125,
    },
  },
  unknown: { "": { label: "", unit: "", min: 0, max: 0 } },
};
