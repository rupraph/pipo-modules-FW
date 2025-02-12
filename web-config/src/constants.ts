import type { InputMinMax } from "./types";

const angleMinMax = {
  min: -180,
  max: 180,
};
const acceleroMinMax = {
  min: -100,
  max: 100,
};
const analogMinMax = {
  min: 0,
  max: 1023,
};
export const minMax: InputMinMax = {
  unknown: {
    "": {
      min: 0,
      max: 0,
    },
  },
  range: {
    dist: {
      // distance in cm
      min: 0,
      max: 3000,
    },
  },
  motion: {
    // acceleration in m/s^2
    accX: acceleroMinMax,
    accY: acceleroMinMax,
    accZ: acceleroMinMax,
    // angles in degrees
    pitch: angleMinMax,
    roll: angleMinMax,
    yaw: angleMinMax,
  },
  analog: {
    A1: analogMinMax,
    A2: analogMinMax,
    A3: analogMinMax,
    A4: analogMinMax,
    A5: analogMinMax,
    A6: analogMinMax,
    // touch input:to be defined, not sure about the range
    T1: analogMinMax,
    T2: analogMinMax,
    T3: analogMinMax,
    T4: analogMinMax,
    T5: analogMinMax,
    T6: analogMinMax,
  },
};
